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
		auto tuple = std::make_tuple(value_AWID, value_AWADDR, value_AWLEN);

		log_action = CHANNEL_ACCEPT;
		log_detail = "AWID=" + std::to_string(value_AWID)
					+ ", AWADDR=" + address_to_hex_string(value_AWADDR)
					+ ", AWLEN=" + std::to_string(value_AWLEN);

		if (std::find(requests_AW.begin(), requests_AW.end(), tuple) != requests_AW.end())
		{
			// Request already exists.
			// Even duplicate, we still need to accept it.
			log_detail += ",DUPLICATE";
		}

		requests_AW.push_back(tuple);

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
		auto tuple = std::make_tuple(value_WID, value_WDATA, value_WLAST);

		log_action = CHANNEL_ACCEPT;
		log_detail = "WID=" + std::to_string(value_WID)
					+ ", WDATA=" + bus_data_to_hex_string(value_WDATA)
					+ ", WLAST=" + std::to_string(value_WLAST);

		if (std::find(requests_W.begin(), requests_W.end(), tuple) != requests_W.end())
		{
			// Request already exists.
			// Even duplicate, we still need to accept it.
			log_detail += ",DUPLICATE";
		}

		requests_W.push_back(tuple);

		WREADY = 0;
		latency_W = LATENCY_READY_W;
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
		auto tuple = std::make_tuple(value_ARID, value_ARADDR, value_ARLEN);

		log_action = CHANNEL_ACCEPT;
		log_detail = "ARID=" + std::to_string(value_ARID)
					+ ", ARADDR=" + address_to_hex_string(value_ARADDR)
					+ ", ARLEN=" + std::to_string(value_ARLEN);

		if (std::find(requests_AR.begin(), requests_AR.end(), tuple) != requests_AR.end())
		{
			// Request already exists.
			// Even duplicate, we still need to accept it.
			log_detail += ",DUPLICATE";
		}

		requests_AR.push_back(tuple);

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

	for (auto tuple_AR: requests_AR)
	{
		uint32_t value_ARID = std::get<0>(tuple_AR);
		uint64_t value_ARADDR = std::get<1>(tuple_AR);
		uint8_t value_ARLEN = std::get<2>(tuple_AR);
		bus_data_t value_RDATA;

		if (map_memory.find(value_ARADDR) != map_memory.end())
		{
			bus_data_t value_RDATA = map_memory[value_ARADDR];
			log_detail = ",RDATA=" + bus_data_to_hex_string(value_RDATA);
			auto tuple_R = std::make_tuple(value_ARID, value_RDATA, 1);
			queue_R.push(tuple_R);
		}
		else
		{
			log_detail = ",NOADDR";
		}

		log_action = CHANNEL_READ;
		log_detail = "ARID=" + std::to_string(value_ARID)
					+ ",ARADDR=" + address_to_hex_string(value_ARADDR)
					+ ",ARLEN=" + std::to_string(value_ARLEN)
					+ log_detail;
	}
	requests_AR.clear();
	channel_log(CHANNEL_NAME_READER, log_action, log_detail);
}

void AXI_SUBORDINATE::channel_writer()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	// Are there jobs to do?
	if (requests_AW.size() == 0 || requests_W.size() == 0)
	{
		// No job for memory write.
		log_action = CHANNEL_IDLE;
		channel_log(CHANNEL_NAME_WRITER, log_action, log_detail);
		return;
	}

	int count_AW = requests_AW.size();
	int count_W = requests_W.size();

	for (auto tuple_W : requests_W)
	{
		uint32_t value_WID = std::get<0>(tuple_W);
		bus_data_t value_WDATA = std::get<1>(tuple_W);
		bool value_WLAST = std::get<2>(tuple_W);

		for (auto tuple_AW : requests_AW)
		{
			uint32_t value_AWID = std::get<0>(tuple_AW);
			uint64_t value_AWADDR = std::get<1>(tuple_AW);
			uint8_t value_AWLEN = std::get<2>(tuple_AW);

			if (value_AWID != value_WID)
			{				
				continue;
			}

			log_action = CHANNEL_WRITE;
			if (map_memory.find(value_AWADDR) != map_memory.end())
			{
				bus_data_t value_WDATA_OLD = map_memory[value_AWADDR];
				log_detail = ",OLD=" + bus_data_to_hex_string(value_WDATA_OLD) + ",OVERWRITE";
			}

			map_memory[value_AWADDR] = value_WDATA;

			log_detail = "WID=" + std::to_string(value_WID)
						+ ",AWADDR=" + address_to_hex_string(value_AWADDR)
						+ ",WDATA=" + bus_data_to_hex_string(value_WDATA)
						+ ",AWLEN=" + std::to_string(value_AWLEN)
						+ ",WLAST=" + std::to_string(value_WLAST)
						+ log_detail;

			// Remove from request queue.
			requests_AW.erase(std::remove(requests_AW.begin(), requests_AW.end(), tuple_AW), requests_AW.end());
			requests_W.erase(std::remove(requests_W.begin(), requests_W.end(), tuple_W), requests_W.end());

			// Add to response queue.
			queue_B.push(value_AWID);
			channel_log(CHANNEL_NAME_WRITER, log_action, log_detail);
			return;
		}

		// code cannot reach here if found.
		log_action = CHANNEL_NO_MATCH;
		log_detail = "WID=" + std::to_string(value_WID)
					+ ",WDATA=" + bus_data_to_hex_string(value_WDATA)
					+ ",WLAST=" + std::to_string(value_WLAST)
					+ "countAW=" + std::to_string(count_AW) + ",countW=" + std::to_string(count_W);
		channel_log(CHANNEL_NAME_WRITER, log_action, log_detail);

		// We can contiue for the next match.
		// Therefore, this may generate many 'NOMATCH' channel logs during single clock cycle.
	}
}

void AXI_SUBORDINATE::on_reset()
{
	event_queue.cancel_all();
	requests_AW.clear();
	requests_W.clear();

	latency_AW = 0;
	latency_W = 0;

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
