#include <fstream>
#include <queue>
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
	sc_out<bool>		WLAST;

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

	const char *filename_access = "m_access.csv";

	std::vector<std::tuple<uint64_t, bus_data_t>> memory;

	// queue access tuple: (timestamp, access_type, address, data)
	std::queue<std::tuple<uint64_t, bus_access_t, uint64_t, bus_data_t>> queue_access;

	// queue AW tuple: (AWID, AWADDR, AWLEN)
	std::queue<std::tuple<uint32_t, uint64_t, uint8_t>> queue_AW;

	// queue W tuple: (WID, WDATA, WLAST)
	std::queue<std::tuple<uint32_t, bus_data_t, bool>> queue_W;

	// queue AR tuple: (ARID, ARADDR, ARLEN)
	std::queue<std::tuple<uint32_t, uint64_t, uint8_t>> queue_AR;

	// queue R tuple: (RID, RDATA, RLAST)
	std::queue<std::tuple<uint32_t, bus_data_t, bool>> queue_R;

	// queue B tuple: (BID, BRESP)
	std::queue<std::tuple<uint32_t, bus_access_t>> queue_B;


	sc_event_queue event_queue;

	SC_CTOR(AXI_MANAGER)
	{
		SC_THREAD(thread_execute);
		sensitive << ACLK << ARESETn << event_queue;
	}

	void channel_manager();
	void channel_AW();
	void channel_W();
	void channel_B();
	void channel_AR();
	void channel_R();

	void channel_log(std::string channel, std::string action, std::string detail);

	void on_reset();
	void on_clock();
	void read_access_csv();
	void write_memory_csv(const char* filename);
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
