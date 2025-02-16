#include <algorithm>
#include <systemc>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <map>

using namespace sc_core;
using namespace sc_dt;

#include "axi_subordinate.h"

void AXI_SUBORDINATE::thread()
{
	on_reset();
	wait();

	while(true)
	{
		on_clock();
		wait();
	}
}

void AXI_SUBORDINATE::on_clock()
{
	channel_writer();
	channel_reader();

	channel_AW();
	channel_W();
	channel_B();
	channel_AR();
	channel_R();
}

void AXI_SUBORDINATE::on_reset()
{
	requests_AR.clear();

	latency_AW = 0;
	latency_W = 0;
	latency_AR = 0;

	is_last_W = false;
	id_last_W = 0;

	AWREADY.write(0);
	WREADY.write(0);
	BVALID.write(0);
	BID.write(0);
	ARREADY.write(0);
	RVALID.write(0);
	RID.write(0);
	RDATA.write(BUS_DATA_ZERO);
	RLAST.write(0);
}

void AXI_SUBORDINATE::channel_AW()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (AWREADY == 0)
	{
		latency_AW --;
		if (latency_AW <= 0)
		{
			AWREADY = 1;
			log_action = CHANNEL_READY;
		}
		else
		{
			log_action = CHANNEL_NOT_READY;
			log_detail = "remain=" + std::to_string(latency_AW);
		}
	}
	else if (AWVALID == 1)	// ready and valid
	{
		uint32_t value_AWID = AWID;
		uint64_t value_AWADDR = AWADDR;
		uint8_t value_AWLEN = AWLEN;

		// tuple_W (addr, length, amount_written_already)
		auto tuple_W = std::make_tuple(value_AWADDR, value_AWLEN, 0);

		auto iter = map_progress_W.find(value_AWID);
		if (iter != map_progress_W.end())
		{
			auto tuple_existing_W = iter->second;
			uint64_t addr_old = std::get<0>(tuple_existing_W);
			log_detail = ",duplicate id, old AWADDR=" + address_to_hex_string(addr_old);
		}
		map_progress_W[value_AWID] = tuple_W;

		log_action = CHANNEL_ACCEPT;
		log_detail = "AWID=" + std::to_string(value_AWID)
					+ ", AWADDR=" + address_to_hex_string(value_AWADDR)
					+ ", AWLEN=" + std::to_string(value_AWLEN);

		AWREADY = 0;
		latency_AW = LATENCY_READY_AW;
	}
	else	// ready but not valid
	{
		log_action = CHANNEL_IDLE;
	}

	channel_log(CHANNEL_NAME_AW, log_action, log_detail);
}

void AXI_SUBORDINATE::channel_W()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (WREADY == 0)
	{
		latency_W --;
		if (latency_W <= 0)
		{
			if (is_last_W)
			{
				queue_B.push(id_last_W);
				is_last_W = false;
				id_last_W = 0;
			}
			WREADY = 1;
			log_action = CHANNEL_READY;
		}
		else
		{
			log_action = CHANNEL_NOT_READY;
			log_detail = "remain=" + std::to_string(latency_W);
		}
	}
	else if (WVALID == 1)	// ready and valid
	{
		uint32_t value_WID = WID;
		bus_data_t value_WDATA = WDATA;
		bool value_WLAST = WLAST;

		auto iter = map_progress_W.find(value_WID);
		if (iter != map_progress_W.end())
		{
			// tuple_W (addr, length, count_written)
			auto tuple_W = iter->second;
			uint64_t addr_begin = std::get<0>(tuple_W);
			uint8_t length = 1 + std::get<1>(tuple_W);
			uint8_t count_written = std::get<2>(tuple_W);
			uint64_t amount_addr_inc = DATA_WIDTH / 8;
			uint64_t addr_to_write = addr_begin + amount_addr_inc * count_written;

			map_memory[addr_to_write] = value_WDATA;

			// update this tuple in the map_progress_W
			count_written ++;
			std::get<2>(iter->second) = count_written;

			log_detail = ", AWADDR=" + address_to_hex_string(addr_begin)
						+ ", addr=" + address_to_hex_string(addr_to_write)
						+ ", part(" + std::to_string(count_written)
						+ "/" + std::to_string(length) + ")";

			uint8_t count_new = std::get<2>(iter->second);
			if (count_new != count_written)
			{
				log_detail += ", count_new=" + std::to_string(count_new);
			}

			if (value_WLAST)
			{
				is_last_W = true;
				id_last_W = value_WID;
				map_progress_W.erase(iter);
			}

		}
		else
		{
			log_detail = ", NOID";
		}

		log_action = CHANNEL_ACCEPT;
		log_detail = "WID=" + std::to_string(value_WID)
					+ ", WDATA=" + bus_data_to_hex_string(value_WDATA)
					+ ", WLAST=" + std::to_string(value_WLAST)
					+ log_detail;

		if (LATENCY_READY_W == 0)
		{
			if (is_last_W)
			{
				queue_B.push(id_last_W);
				is_last_W = false;
				id_last_W = 0;
			}
		}
		else
		{
			WREADY = 0;
			latency_W = LATENCY_READY_W;
		}
	}
	else	// ready but not valid
	{
		log_action = CHANNEL_IDLE;
	}
	channel_log(CHANNEL_NAME_W, log_action, log_detail);
}

void AXI_SUBORDINATE::channel_B()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (BVALID == 0)
	{
		if (queue_B.empty())
		{
			// No job to do.
			log_action = CHANNEL_IDLE;
			log_detail = "";
		}
		else
		{
			uint32_t value_BID = queue_B.front();

			BVALID = 1;
			BID = value_BID;

			queue_B.pop();
			log_action = CHANNEL_INITIATE;
			log_detail = "BID=" + std::to_string(value_BID);
		}
	}
	else
	{
		if (BREADY == 1)
		{
			BVALID = 0;
			BID = 0;
			log_action = CHANNEL_COMPLETE;
		}
		else
		{
			log_action = CHANNEL_WAIT;
		}
	}
	channel_log(CHANNEL_NAME_B, log_action, log_detail);
}


void AXI_SUBORDINATE::channel_AR()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (ARREADY == 0)
	{
		latency_AR --;
		if (latency_AR <= 0)
		{
			if (has_delayed_tuple_AR)
			{
				requests_AR.push_back(delayed_tuple_AR);
				has_delayed_tuple_AR = false;
			}

			ARREADY = 1;
			log_action = CHANNEL_READY;
		}
		else
		{
			log_action = CHANNEL_NOT_READY;
			log_detail = "remain=" + std::to_string(latency_AR);
		}
	}
	else if (ARVALID == 1)	// ready and valid
	{
		uint32_t value_ARID = ARID;
		uint64_t value_ARADDR = ARADDR;
		uint8_t value_ARLEN = ARLEN;
		auto tuple_AR = std::make_tuple(value_ARID, value_ARADDR, value_ARLEN);

		log_action = CHANNEL_ACCEPT;
		log_detail = "ARID=" + std::to_string(value_ARID)
					+ ", ARADDR=" + address_to_hex_string(value_ARADDR)
					+ ", ARLEN=" + std::to_string(value_ARLEN);

		if (std::find(requests_AR.begin(), requests_AR.end(), tuple_AR) != requests_AR.end())
		{
			// Request already exists.
			// Even duplicate, we still need to accept it.
			log_detail += ",DUPLICATE";
		}

		has_delayed_tuple_AR = true;
		delayed_tuple_AR = tuple_AR;

		ARREADY = 0;
		latency_AR = LATENCY_READY_AR;
	}
	else	// ready but not valid
	{
		log_action = CHANNEL_IDLE;
	}

	channel_log(CHANNEL_NAME_AR, log_action, log_detail);
}

void AXI_SUBORDINATE::channel_R()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (RVALID == 0)
	{
		if (queue_R.empty())
		{
			// No job to do.
			log_action = CHANNEL_IDLE;
			log_detail = "";
		}
		else
		{
			auto tuple = queue_R.front();
			uint32_t value_RID = std::get<0>(tuple);
			bus_data_t value_RDATA = std::get<1>(tuple);
			bool value_RLAST = std::get<2>(tuple);

			RVALID = 1;
			RID = value_RID;
			RDATA = value_RDATA;
			RLAST = value_RLAST;

			queue_R.pop();

			log_action = CHANNEL_INITIATE;
			log_detail = "RID=" + std::to_string(value_RID),
						+ "RDATA=" + bus_data_to_hex_string(value_RDATA)
						+ "RLAST=" = std::to_string(value_RLAST);
		}
	}
	else
	{
		if (RREADY == 1)
		{
			RVALID = 0;
			RID = 0;
			RDATA = 0;
			RLAST = 0;
			log_action = CHANNEL_COMPLETE;
		}
		else
		{
			log_action = CHANNEL_WAIT;
		}
	}
	channel_log(CHANNEL_NAME_R, log_action, log_detail);
}

void AXI_SUBORDINATE::channel_log(std::string channel, std::string action, std::string detail)
{
	std::string out;
	out = sc_time_stamp().to_string() + ":SUBORDINATE:" + channel + ":" + action + ":" + detail;
	std::cout << out << std::endl;
}

void AXI_SUBORDINATE::channel_reader()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	// Are there jobs to do?
	if (requests_AR.size() == 0)
	{
		// No job for memory read
		log_action = CHANNEL_IDLE;
		channel_log(CHANNEL_NAME_READER, log_action, log_detail);
		return;
	}

	// tuple_AR (ARID, ARADDR, ARLEN)
	for (auto tuple_AR: requests_AR)
	{
		uint32_t value_ARID = std::get<0>(tuple_AR);
		uint64_t value_ARADDR = std::get<1>(tuple_AR);
		uint8_t value_ARLEN = std::get<2>(tuple_AR);

		uint64_t addr_to_read = value_ARADDR;
		uint8_t count_addr_read = 0;
		uint64_t amount_addr_increase = DATA_WIDTH / 8;
		bus_data_t data_to_read;
		bool is_last = false;

		do
		{
			if (count_addr_read == value_ARLEN)
			{
				is_last = true;
			}

			if (map_memory.find(addr_to_read) != map_memory.end())
			{
				data_to_read = map_memory[addr_to_read];
				log_detail += bus_data_to_hex_string(data_to_read);
	
				// tuple_R (RID, RDATA, RLAST)
				auto tuple_R = std::make_tuple(value_ARID, data_to_read, is_last);
				queue_R.push(tuple_R);
			}
			else
			{
				log_detail += "NODATA";
			}

			if (is_last == false)
			{
				log_detail += ", ";
			}
	
			// prepare for the next read
			count_addr_read ++;
			addr_to_read += amount_addr_increase;

		} while (count_addr_read <= value_ARLEN);

		log_action = CHANNEL_READ;
		log_detail = "ARID=" + std::to_string(value_ARID)
					+ ", ARADDR=" + address_to_hex_string(value_ARADDR)
					+ ", ARLEN=" + std::to_string(value_ARLEN)
					+ ", data=" + log_detail;
		channel_log(CHANNEL_NAME_READER, log_action, log_detail);
	}

	requests_AR.clear();
}

void AXI_SUBORDINATE::channel_writer()
{
	// Are there jobs to do?
}

void AXI_SUBORDINATE::read_memory_csv()
{
	map_memory.clear();

	std::ifstream f(filename_memory);
	if (!f.is_open())
	{
		std::cerr << "Error: could not open " << filename_memory << std::endl;
		return;
	}

	int line_number = 0;
	std::string line;
	while (std::getline(f, line))
	{
		std::tuple<uint64_t, bus_data_t> row;
		std::istringstream iss(line);
		uint64_t address;
		bus_data_t data;
		std::string token1, token2;
		std::getline(iss, token1, ',');
		std::getline(iss, token2, ',');
		if (token1.empty() || token2.empty())
		{
			std::cerr << "Error: invalid format in " << filename_memory << std::endl;
			std::cerr << "At line (" << line_number << "): " << line << std::endl;
			continue;
		}
		address = address_from_hex_string(token1);
		data = bus_data_from_hex_string(token2);
		map_memory.insert(std::make_pair(address, data));
		line_number ++;
	}
}

void AXI_SUBORDINATE::write_memory_csv(const char *filename)
{
	std::ofstream f(filename);
	if (!f.is_open())
	{
		std::cerr << "Error: could not open " << filename << std::endl;
		return;
	}

	std::map<uint64_t, bus_data_t> map_ordered(map_memory.begin(), map_memory.end());
	for (auto row : map_ordered)
	{
		uint64_t address = std::get<0>(row);
		bus_data_t data = std::get<1>(row);
		f << "0x" << std::setfill('0') << std::setw(ADDR_WIDTH / 4) << std::hex << address;
		f << ","  << std::setfill('0') << std::setw(DATA_WIDTH / 4) << data.to_string(SC_HEX) << std::endl;
	}
}

