#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include "axi_param.h"

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
	sc_in<bool>			WLAST;

	// Chapter A2.1.3 write response channel
	sc_out<bool>		BVALID;
	sc_in<bool>			BREADY;
	sc_out<uint32_t>	BID;

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

	// pair<address, data>
	std::unordered_map<uint64_t, bus_data_t> map_memory;

	// tuple<AWID, AWADDR, AWLEN>
	std::vector<std::tuple<uint32_t, uint64_t, uint8_t>> requests_AW;

	// tuple<WID, WDATA, WLAST>
	std::vector<std::tuple<uint32_t, bus_data_t, bool>> requests_W;

	// tuple<BID, BRESP>
	std::queue<uint32_t> queue_B;

	// tuple<ARID, ARADDR, ARLEN>
	std::vector<std::tuple<uint32_t, uint64_t, bool>> requests_AR;

	// tuple<RID, RDATA, RLAST>
	std::queue<std::tuple<uint32_t, bus_data_t, bool>> queue_R;

	// for latency cowntdown
	int latency_AW;
	int latency_W;
	int latency_AR;
	int latency_R;

	const char *filename_memory = "s_memory.csv";

	sc_event_queue event_queue;

	SC_CTOR(AXI_SUBORDINATE)
	{
		SC_THREAD(thread_execute);
		sensitive << ACLK << ARESETn << event_queue;
	}

	void channel_writer();
	void channel_reader();
	void channel_AW();
	void channel_W();
	void channel_B();
	void channel_AR();
	void channel_R();

	void channel_log(std::string channel, std::string action, std::string detail);

	void on_reset();
	void on_clock();
	void read_memory_csv();
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
