#include <systemc>
#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
#include <string>
#include <functional>

#include "axi_param.h"

typedef struct
{
	int			channel;
	uint32_t	id;
	uint64_t	addr;
	uint8_t		addr_len;
	bus_data_t	data;
	bool		is_last;

	// for fifo
	bool		is_write;
} axi_info_t;

//typedef int (*axi_callback_t) (AXI_BUS* bus, int channel, int when, axi_info_t info);

SC_MODULE(AXI_BUS)
{
	sc_in<bool>	ACLK;
	sc_in<bool>	ARESETn;

	sc_fifo_in<axi_info_t> fifo_in_M;
	sc_fifo_out<axi_info_t> fifo_out_M;
	sc_fifo_in<axi_info_t> fifo_in_S;
	sc_fifo_out<axi_info_t> fifo_out_S;

	// Chapter A2.1.1 write request channel
	sc_signal<bool>			AWVALID;
	sc_signal<bool>			AWREADY;
	sc_signal<uint32_t>		AWID;
	sc_signal<uint64_t>		AWADDR;
	sc_signal<uint8_t>		AWLEN;

	// Chapter A2.1.2 write data channel
	sc_signal<bool>			WVALID;
	sc_signal<bool>			WREADY;
	sc_signal<uint32_t>		WID;
	sc_signal<bus_data_t>	WDATA;
	sc_signal<bool>			WLAST;

	// Chapter A2.1.3 write response channel
	sc_signal<bool>			BVALID;
	sc_signal<bool>			BREADY;
	sc_signal<uint32_t>		BID;

	// Chapter A2.2.1 read request channel
	sc_signal<bool>			ARVALID;
	sc_signal<bool>			ARREADY;
	sc_signal<uint32_t>		ARID;
	sc_signal<uint64_t>		ARADDR;
	sc_signal<uint8_t>		ARLEN;

	// Chapter A2.2.2 read data channel
	sc_signal<bool>			RVALID;
	sc_signal<bool>			RREADY;
	sc_signal<uint32_t>		RID;
	sc_signal<bus_data_t>	RDATA;
	sc_signal<bool>			RLAST;

	std::queue<axi_info_t> q_send_AW;
	std::queue<axi_info_t> q_send_W;
	std::queue<axi_info_t> q_send_B;
	std::queue<axi_info_t> q_send_AR;
	std::queue<axi_info_t> q_send_R;

	std::queue<axi_info_t> q_recv_AW;
	std::queue<axi_info_t> q_recv_W;
	std::queue<axi_info_t> q_recv_B;
	std::queue<axi_info_t> q_recv_AR;
	std::queue<axi_info_t> q_recv_R;

	SC_CTOR(AXI_BUS)
	{
		SC_CTHREAD(thread, ACLK);
		async_reset_signal_is(ARESETn, false);
//		callback = callback_default;
	}

	void thread();
	void on_clock();
	void on_reset();

//	axi_callback_t callback;
//	int callback_default (AXI_BUS* bus, int channel, int event, axi_info_t info);

	axi_info_t create_null_info();
	void send_info(int channel, axi_info_t info);
	axi_info_t recv_info(int channel);

	static std::string get_channel_name(int channel);
	bool is_ready(int channel);
	bool is_valid(int channel);
	void set_ready(int channel, bool value);
	void set_valid(int channel, bool value);

	void channel_transaction();
	void fifo_transaction();

	void fifo_transaction_in_M();
	void fifo_transaction_in_S();
	void fifo_transaction_out_M();
	void fifo_transaction_out_S();

	void channel_sender(int channel, std::queue<axi_info_t> q);
	void channel_receiver(int channel, std::queue<axi_info_t> q);

	void log(int channel, std::string action, std::string detail);

	uint32_t generate_transaction_id();

};
