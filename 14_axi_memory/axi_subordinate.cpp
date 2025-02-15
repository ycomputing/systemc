#include <algorithm>
#include <systemc>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>

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
			log_detail = "LATENCY=" + std::to_string(latency_AW);
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
			log_detail += ", DUPLICATE";
		}
		else
		{
			requests_AW.push_back(tuple);
		}

		AWREADY = 0;
		latency_AW = LATENCY_READY_AW;
	}
	else	// ready but not valid
	{
		log_action = CHANNEL_IDLE;
	}

	channel_log(CHANNEL_AW, log_action, log_detail);
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
			log_detail = "LATENCY=" + std::to_string(latency_W);
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
			log_detail += ", DUPLICATE";
		}
		else
		{
			requests_W.push_back(tuple);
		}

		WREADY = 0;
		latency_W = LATENCY_READY_W;
	}
	else	// ready but not valid
	{
		log_action = CHANNEL_IDLE;
	}
	channel_log(CHANNEL_W, log_action, log_detail);
}

void AXI_SUBORDINATE::channel_log(std::string channel, std::string action, std::string detail)
{
	std::string out;
	out = sc_time_stamp().to_string() + ":SUBORDINATE:" + channel + ":" + action + ":" + detail;
	std::cout << out << std::endl;
}

void AXI_SUBORDINATE::channel_manager()
{
	// Are there jobs to do?
	// Are there jobs done?
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
	BRESP.write(0);
	ARREADY.write(0);
	RVALID.write(0);
	RID.write(0);
	RDATA.write(BUS_DATA_ZERO);
	RLAST.write(0);
}

void AXI_SUBORDINATE::read_memory_csv()
{
	memory.clear();

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
		address = std::stoull(token1, nullptr, 16);
		data = bus_data_from_hex_string(token2);
		row = std::make_tuple(address, data);
		memory.push_back(row);
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

	std::sort(memory.begin(), memory.end());

	for (auto row : memory)
	{
		uint64_t address = std::get<0>(row);
		bus_data_t data = std::get<1>(row);
		f << "0x" << std::setfill('0') << std::setw(ADDR_WIDTH / 4) << std::hex << address;
		f << ","  << std::setfill('0') << std::setw(DATA_WIDTH / 4) << data.to_string(SC_HEX) << std::endl;
	}
}

void AXI_SUBORDINATE::on_clock()
{
	channel_manager();
	channel_AW();
	channel_W();
}
