#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "axi_param.h"

SC_MODULE(AXI_MANAGER)
{
	sc_in<bool>	ACLK;
	sc_in<bool>	ARESETn;

	// Chapter A2.1.1 write request channel
	sc_out<bool>		AWVALID;
	sc_in<bool>			AWREADY;
	sc_out<uint32_t>	AWID;
	sc_out<uint64_t>	AWADDR;
	sc_out<uint8_t>		AWLEN;

	// Chapter A2.1.2 write data channel
	sc_out<bool>		WVALID;
	sc_in<bool>			WREADY;
	sc_out<uint32_t>	WID;
	sc_out<bus_data_t>	WDATA;

	// Chapter A2.1.3 write response channel
	sc_in<bool>			BVALID;
	sc_out<bool>		BREADY;
	sc_in<uint32_t>		BID;
	sc_in<bool>			BRESP;

	// Chapter A2.2.1 read request channel
	sc_out<bool>		ARVALID;
	sc_in<bool>			ARREADY;
	sc_out<uint32_t>	ARID;
	sc_out<uint64_t>	ARADDR;
	sc_out<uint8_t>		ARLEN;

	// Chapter A2.2.2 read data channel
	sc_in<bool>			RVALID;
	sc_out<bool>		RREADY;
	sc_in<uint32_t>		RID;
	sc_in<bus_data_t>	RDATA;
	sc_in<bool>			RLAST;

	const char *filename = "random_access.csv";
	std::vector<std::tuple<uint64_t, bus_data_t>> memory;
	std::vector<std::tuple<uint64_t, bus_access_t, uint64_t, bus_data_t>> access;
	sc_event_queue event_queue;

	SC_CTOR(AXI_MANAGER)
	{
		SC_THREAD(thread_execute);
		sensitive << ACLK << ARESETn << event_queue;
	}

	void on_reset();
	void read_access_csv();
	void on_clock();
	void thread_execute()
	{
		while(true)
		{
			if (ARESETn == 0)
			{
				on_reset();
			}
			else if (ACLK.posedge())
			{
				on_clock();
			}
			wait();			
		}
	}
};
