#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include "axi_param.h"

#define READ_LATENCY	10
#define WRITE_LATENCY	10

SC_MODULE(AXI_SUBORDINATE)
{
	sc_in<bool>	ACLK;
	sc_in<bool>	ARESETn;

	// Chapter A2.1.1 write request channel
	sc_in<bool>			AWVALID;
	sc_out<bool>		AWREADY;
	sc_in<uint32_t>		AWID;
	sc_in<uint64_t>		AWADDR;
	sc_in<uint8_t>		AWLEN;

	// Chapter A2.1.2 write data channel
	sc_in<bool>			WVALID;
	sc_out<bool>		WREADY;
	sc_in<uint32_t>		WID;
	sc_in<bus_data_t>	WDATA;

	// Chapter A2.1.3 write response channel
	sc_out<bool>		BVALID;
	sc_in<bool>			BREADY;
	sc_out<uint32_t>	BID;
	sc_out<bool>		BRESP;

	// Chapter A2.2.1 read request channel
	sc_in<bool>			ARVALID;
	sc_out<bool>		ARREADY;
	sc_in<uint32_t>		ARID;
	sc_in<uint64_t>		ARADDR;
	sc_in<uint8_t>		ARLEN;

	// Chapter A2.2.2 read data channel
	sc_out<bool>		RVALID;
	sc_in<bool>			RREADY;
	sc_out<uint32_t>	RID;
	sc_out<bus_data_t>	RDATA;
	sc_out<bool>		RLAST;

	std::vector<std::tuple<uint64_t, bus_data_t>> memory;

	const char *filename = "s_memory.csv";
	sc_event_queue event_queue;

	SC_CTOR(AXI_SUBORDINATE)
	{
		SC_THREAD(thread_execute);
		sensitive << ACLK << ARESETn << event_queue;
	}

	void on_reset();
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
