#include <iostream>
#include <systemc>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <map>

using namespace sc_core;
using namespace sc_dt;

#include "axi_manager.h"

void AXI_MANAGER::on_reset()
{
	event_queue.cancel_all();

	AWVALID.write(0);
	AWID.write(0);
	AWADDR.write(0);
	AWLEN.write(0);

	WVALID.write(0);
	WID.write(0);
	WDATA.write(BUS_DATA_ZERO);
	WLAST.write(0);

	BREADY.write(0);

	ARVALID.write(0);
	ARID.write(0);
	ARADDR.write(0);
	ARLEN.write(0);

	RREADY.write(0);

}

void AXI_MANAGER::channel_manager()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";
	
	// Are there jobs to do?

	if (queue_access.empty())
	{
		// No job to do.
		log_action = CHANNEL_IDLE;
		channel_log(CHANNEL_NAME_WRITER, log_action, log_detail);
		return;
	}

	// queue access tuple: (timestamp, rw, address, length, data)
	auto tuple = queue_access.front();
	uint64_t value_stamp = std::get<0>(tuple);
	char value_rw = std::get<1>(tuple);
	uint64_t value_address = std::get<2>(tuple);
	uint8_t value_length = std::get<3>(tuple);
	bus_data_t value_data = std::get<4>(tuple);

	// XXX Warning:
	// sc_time_stamp().value() depends on the time resolution of the simulation.
	// It is not guaranteed to be the same as the timestamp in the CSV file.
	// So we assume that the timestamp in the CSV file is the NS(nano seconds),
	// whereas the timestamp in the simulation is the PS(pico seconds).

	uint64_t current_stamp = sc_time_stamp().value() / 1000; // ps to ns convert.

	if (value_stamp > current_stamp)
	{
		// The time has not come yet
		log_action = CHANNEL_HOLD;
		log_detail = "scheduled=" + std::to_string(value_stamp)
				+ ",now=" + std::to_string(current_stamp);
		channel_log(CHANNEL_NAME_MANAGER, log_action, log_detail);
		return;
	}

	queue_access.pop();

	log_action = CHANNEL_ENQUEUE;
	log_detail = "stamp=" + std::to_string(value_stamp)
			+ ", rw=" + std::string{value_rw}
			+ ", address=" + address_to_hex_string(value_address)
			+ ", lendth=" + std::to_string(value_length)
			+ ", data=" + bus_data_to_hex_string(value_data);

	uint32_t transaction_id = generate_transaction_id();

	if (value_rw == BUS_ACCESS_READ)
	{
		// tuple(ARID, ARADDR, ARLEN)
		queue_AR.push(std::make_tuple(transaction_id, value_address, value_length - 1));

		for (int follow = 0; follow < value_length - 1; follow ++)
		{
			// Ignore [length - 1] following entries.
			// TODO: check whether ignored entries maintain access pattern integrity
			queue_access.pop();
		}
	}
	else if (value_rw == BUS_ACCESS_WRITE)
	{
		// tuple(AWID, AWADDR, AWLEN)
		queue_AW.push(std::make_tuple(transaction_id, value_address, value_length - 1));

		// Prepare next [length - 1] following entries, if any
		for (int follow = 0; follow < value_length - 1; follow ++)
		{
			// Enqueue the existing entry
			// tuple(WID, WDATA, WLAST)
			queue_W.push(std::make_tuple(transaction_id, value_data, false));

			// TODO: check whether ignored entries maintain access pattern integrity
			auto follow_tuple = queue_access.front();
			//uint64_t ignore_stamp = std::get<0>(tuple);
			//char ignore_rw = std::get<1>(tuple);
			//uint64_t ignore_address = std::get<2>(tuple);
			//uint8_t ignore_length = std::get<3>(tuple);
			bus_data_t value_data = std::get<4>(tuple);
			log_detail += "," + bus_data_to_hex_string(value_data);

			queue_access.pop();
		}

		// This is the last one, WLAST is true
		// tuple(WID, WDATA, WLAST)
		queue_W.push(std::make_tuple(transaction_id, value_data, true));
	}
	else
	{
		// This must not happen
		log_detail += ",INVALID ACCESS TYPE";
		channel_log(CHANNEL_NAME_MANAGER, log_action, log_detail);
		SC_REPORT_ERROR("AXI_MANAGER", "Invalid access type");
	}

	channel_log(CHANNEL_NAME_MANAGER, log_action, log_detail);
}

void AXI_MANAGER::channel_log(std::string channel, std::string action, std::string detail)
{
	std::string out;
	out = sc_time_stamp().to_string() + ":MANAGER:" + channel + ":" + action + ":" + detail;
	std::cout << out << std::endl;
}

void AXI_MANAGER::channel_AW()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (AWVALID == 0)
	{
		if (queue_AW.empty())
		{
			// No job to do.
			log_action = CHANNEL_IDLE;
			log_detail = "";
		}
		else
		{
			auto tuple = queue_AW.front();
			uint32_t value_AWID = std::get<0>(tuple);
			uint64_t value_AWADDR = std::get<1>(tuple);
			uint8_t value_AWLEN = std::get<2>(tuple);

			AWVALID = 1;
			AWID = value_AWID;
			AWADDR = value_AWADDR;
			AWLEN = value_AWLEN;

			queue_AW.pop();
			log_action = CHANNEL_INITIATE;
			log_detail = "AWID=" + std::to_string(value_AWID)
					+ ", AWADDR=" + address_to_hex_string(value_AWADDR)
					+ ", AWLEN=" + std::to_string(value_AWLEN);
		}
	}
	else
	{
		if (AWREADY == 1)
		{
			AWVALID = 0;
			AWID = 0;
			AWADDR = 0;
			AWLEN = 0;
			log_action = CHANNEL_COMPLETE;
		}
		else
		{
			log_action = CHANNEL_WAIT;
		}
	}
	channel_log(CHANNEL_NAME_AW, log_action, log_detail);
}

void AXI_MANAGER::channel_W()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (WVALID == 0)
	{
		if (queue_W.empty())
		{
			// No job to do.
			log_action = CHANNEL_IDLE;
			log_detail = "";
		}
		else
		{
			auto tuple = queue_W.front();
			uint32_t value_WID = std::get<0>(tuple);
			bus_data_t value_WDATA = std::get<1>(tuple);
			bool value_WLAST = std::get<2>(tuple);

			WVALID = 1;
			WID = value_WID;
			WDATA = value_WDATA;
			WLAST = value_WLAST;

			queue_W.pop();
			log_action = CHANNEL_INITIATE;
			log_detail = "WID=" + std::to_string(value_WID)
					+ ", WDATA=" + bus_data_to_hex_string(value_WDATA)
					+ ", WLAST=" + std::to_string(value_WLAST);
		}
	}
	else
	{
		if (WREADY == 1)
		{
			WVALID = 0;
			WID = 0;
			WDATA = BUS_DATA_ZERO;
			WLAST = 0;

			log_action = CHANNEL_COMPLETE;
		}
		else
		{
			log_action = CHANNEL_WAIT;
		}
	}
	channel_log(CHANNEL_NAME_W, log_action, log_detail);
}

void AXI_MANAGER::channel_B()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (BREADY == 0)
	{
		latency_B --;
		if (latency_B <= 0)
		{
			BREADY = 1;
			log_action = CHANNEL_READY;
		}
		else
		{
			log_action = CHANNEL_NOT_READY;
			log_detail = "remain=" + std::to_string(latency_B);
		}
	}
	else if (BVALID == 1)	// ready and valid
	{
		uint32_t value_BID = BID;
		BREADY = 0;
		latency_B = LATENCY_READY_B;
		log_action = CHANNEL_ACCEPT;
		log_detail = "BID=" + std::to_string(value_BID);
	}
	else	// ready but not valid
	{
		log_action = CHANNEL_IDLE;
	}
	channel_log(CHANNEL_NAME_B, log_action, log_detail);
}

void AXI_MANAGER::channel_AR()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (ARVALID == 0)
	{
		if (queue_AR.empty())
		{
			// No job to do.
			log_action = CHANNEL_IDLE;
			log_detail = "";
		}
		else
		{
			auto tuple = queue_AR.front();
			uint32_t value_ARID = std::get<0>(tuple);
			uint64_t value_ARADDR = std::get<1>(tuple);
			uint8_t value_ARLEN = std::get<2>(tuple);

			ARVALID = 1;
			ARID = value_ARID;
			ARADDR = value_ARADDR;
			ARLEN = value_ARLEN;

			queue_AR.pop();

			auto existing = map_requests_AR.find(value_ARID);
			if (existing != map_requests_AR.end())
			{
				uint64_t old_ARADDR = existing->first;
				log_detail = ",duplicate id, old ARADDR=" + address_to_hex_string(old_ARADDR);
			}
			map_requests_AR[value_ARID] = value_ARADDR;

			log_action = CHANNEL_INITIATE;
			log_detail = "ARID=" + std::to_string(value_ARID)
					+ ", ARADDR=" + address_to_hex_string(value_ARADDR)
					+ ", ARLEN=" + std::to_string(value_ARLEN)
					+ log_detail;
		}
	}
	else
	{
		if (ARREADY == 1)
		{
			ARVALID = 0;
			ARID = 0;
			ARADDR = 0;
			ARLEN = 0;
			log_action = CHANNEL_COMPLETE;
		}
		else
		{
			log_action = CHANNEL_WAIT;
		}
	}
	channel_log(CHANNEL_NAME_AR, log_action, log_detail);
}

void AXI_MANAGER::channel_R()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (RREADY == 0)
	{
		latency_R --;
		if (latency_R <= 0)
		{
			RREADY = 1;
			log_action = CHANNEL_READY;
		}
		else
		{
			log_action = CHANNEL_NOT_READY;
			log_detail = "remain=" + std::to_string(latency_R);
		}
	}
	else if (RVALID == 1)	// ready and valid
	{
		uint32_t value_RID = RID;
		bus_data_t value_RDATA = RDATA;
		bool value_RLAST = RLAST;

		auto iter = map_requests_AR.find(value_RID);
		if (iter != map_requests_AR.end())
		{
			uint64_t addr = iter->second;
			map_memory[addr] = value_RDATA;
			map_requests_AR.erase(iter);
			log_detail = ",ADDR=" + address_to_hex_string(addr);
		}
		else
		{
			log_detail = ",NOID";
		}

		log_action = CHANNEL_ACCEPT;
		log_detail = "RID=" + std::to_string(value_RID)
					+ ", RDATA=" + bus_data_to_hex_string(value_RDATA)
					+ ", RLAST=" + std::to_string(value_RLAST)
					+ log_detail;

		RREADY = 0;
		latency_R = LATENCY_READY_R;
	}
	else	// ready but not valid
	{
		log_action = CHANNEL_IDLE;
	}
	channel_log(CHANNEL_NAME_R, log_action, log_detail);
}

uint32_t AXI_MANAGER::generate_transaction_id()
{
	static uint32_t id = 0;
	id ++;
	return id;
}

void AXI_MANAGER::read_access_csv()
{
	while (!queue_access.empty())
	{
		queue_access.pop();
	}

	std::ifstream f(filename_access);
	if (!f.is_open())
	{
		std::cerr << "Error: could not open " << filename_access << std::endl;
		return;
	}

	int line_number = 0;
	std::string line;
	while (std::getline(f, line))
	{
		std::istringstream iss(line);
		u_int64_t stamp;
		char rw;
		uint64_t address;
		uint8_t length;
		bus_data_t data;
		std::string token1, token2, token3, token4, token5;

		// line format
		// stamp, R/W, address, length, data
		//
		// stamp: integer, simulation time in nano second
		// R/W: character, 'R' or 'W', to indicate read or write action
		// address: hex string, 64 bit, address to read or write
		// length: integer, how many data to transfer at one transaction
		// data: hex string, 128 bit, data to transfer

		std::tuple<uint64_t, char, uint64_t, uint8_t, bus_data_t> row;

		std::getline(iss, token1, ',');
		std::getline(iss, token2, ',');
		std::getline(iss, token3, ',');
		std::getline(iss, token4, ',');
		std::getline(iss, token5, ',');
		if (token1.empty() || token2.empty() || token3.empty() || token4.empty() || token5.empty())
		{
			std::cerr << "Error: invalid format in " << filename_access << std::endl;
			std::cerr << "At line (" << line_number << "): " << line << std::endl;
			continue;
		}
		stamp = std::stoull(token1);
		rw = token2[0];
		address = std::stoull(token3, nullptr, 16);
		length = std::stoul(token4);
		data = token5.c_str();
		row = std::make_tuple(stamp, rw, address, length, data);
		queue_access.push(row);
		line_number ++;
	}
}

void AXI_MANAGER::write_memory_csv(const char *filename)
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

void AXI_MANAGER::on_clock()
{
	channel_manager();

	channel_AW();
	channel_W();
	channel_B();
	channel_AR();
	channel_R();
}
