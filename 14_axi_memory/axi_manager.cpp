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
	memory.clear();
	access.clear();
	read_access_csv();

	AWVALID.write(0);
	AWID.write(0);
	AWADDR.write(0);
	AWLEN.write(0);

	WVALID.write(0);
	WID.write(0);
	WDATA.write(BUS_DATA_ZERO);

	BREADY.write(0);

	ARVALID.write(0);
	ARID.write(0);
	ARADDR.write(0);
	ARLEN.write(0);

	RREADY.write(0);

}

void AXI_MANAGER::read_access_csv()
{
	std::ifstream f(filename);
	if (!f.is_open())
	{
		std::cerr << "Error: could not open " << filename << std::endl;
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
			std::cerr << "Error: invalid format in " << filename << std::endl;
			std::cerr << "At line (" << line_number << "): " << line << std::endl;
			continue;
		}
		stamp = std::stoull(token1);
		rw = (bus_access_t) token2[0];
		address = std::stoull(token3, nullptr, 16);
		data = ("0x" + token4).c_str();
		row = std::make_tuple(stamp, rw, address, data);
		access.push_back(row);
		line_number ++;
	}
}

void AXI_MANAGER::on_clock()
{

}
