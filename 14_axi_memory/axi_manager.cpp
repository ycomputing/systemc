#include <iostream>
#include <systemc>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

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
	// Are there jobs to do?

	if (queue_access.empty())
	{
		// No job to do.
		return;
	}

	// queue access tuple: (timestamp, access_type, address, data)
	auto tuple = queue_access.front();
	uint64_t value_stamp = std::get<0>(tuple);
	bus_access_t value_rw = std::get<1>(tuple);
	uint64_t value_address = std::get<2>(tuple);
	bus_data_t value_data = std::get<3>(tuple);

	uint64_t current_stamp = sc_time_stamp().value() / 1000; // ps to ns convert.
	if (value_stamp <= current_stamp)
	{
		queue_access.pop();
		if (value_rw == BUS_ACCESS_READ)
		{
			queue_AR.push(std::make_tuple(0, value_address, 0));
			queue_R.push(std::make_tuple(0, BUS_DATA_ZERO, 0));
		}
		else if (value_rw == BUS_ACCESS_WRITE)
		{
			queue_AW.push(std::make_tuple(0, value_address, 0));
			queue_W.push(std::make_tuple(0, value_data, 0));
		}
		else
		{
			SC_REPORT_ERROR("AXI_MANAGER", "Invalid access type");
		}
	}
	// Are there jobs done?
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
	channel_log(CHANNEL_AW, log_action, log_detail);
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
	channel_log(CHANNEL_W, log_action, log_detail);
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
		std::tuple<uint64_t, bus_access_t, uint64_t, bus_data_t> row;
		std::istringstream iss(line);
		u_int64_t stamp;
		bus_access_t rw;
		uint64_t address;
		bus_data_t data;
		std::string token1, token2, token3, token4;
		std::getline(iss, token1, ',');
		std::getline(iss, token2, ',');
		std::getline(iss, token3, ',');
		std::getline(iss, token4, ',');
		if (token1.empty() || token2.empty() || token3.empty() || token4.empty())
		{
			std::cerr << "Error: invalid format in " << filename_access << std::endl;
			std::cerr << "At line (" << line_number << "): " << line << std::endl;
			continue;
		}
		stamp = std::stoull(token1);
		rw = (bus_access_t) token2[0];
		address = std::stoull(token3, nullptr, 16);
		data = token4.c_str();
		row = std::make_tuple(stamp, rw, address, data);
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

	for (auto row : memory)
	{
		f << std::hex << std::get<0>(row) << "," << std::get<1>(row) << std::endl;
	}
}

void AXI_MANAGER::on_clock()
{
	channel_manager();
	channel_AW();
	channel_W();
}
