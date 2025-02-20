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

#include "axi_bus.h"

void AXI_BUS::thread()
{
	on_reset();
	wait();

	while(true)
	{
		on_clock();
		wait();
	}
}

void AXI_BUS::on_clock()
{
	fifo_transaction_in_M();
	fifo_transaction_in_S();

	channel_transaction();

	fifo_transaction_out_M();
	fifo_transaction_out_S();
}

void AXI_BUS::on_reset()
{
	AWVALID.write(0);
	AWREADY.write(0);
	AWID.write(0);
	AWADDR.write(0);
	AWLEN.write(0);

	WVALID.write(0);
	WREADY.write(0);
	WID.write(0);
	WDATA.write(0);
	WLAST.write(0);

	BVALID.write(0);
	BREADY.write(0);
	BID.write(0);

	ARVALID.write(0);
	ARREADY.write(0);
	ARID.write(0);
	ARADDR.write(0);
	ARLEN.write(0);

	RVALID.write(0);
	RREADY.write(0);
	RID.write(0);
	RDATA.write(0);
	RLAST.write(0);
}

bool AXI_BUS::is_ready(int channel)
{
	switch (channel)
	{
		case CHANNEL_AW:	return(AWREADY);
		case CHANNEL_W:		return(WREADY);
		case CHANNEL_B:		return(BREADY);
		case CHANNEL_AR:	return(ARREADY);
		case CHANNEL_R:		return(RREADY);
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return false;
	}
}

bool AXI_BUS::is_valid(int channel)
{
	switch (channel)
	{
		case CHANNEL_AW:	return(AWVALID);
		case CHANNEL_W:		return(WVALID);
		case CHANNEL_B:		return(BVALID);
		case CHANNEL_AR:	return(ARVALID);
		case CHANNEL_R:		return(RVALID);
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return false;
	}
}

void AXI_BUS::set_valid(int channel, bool value)
{
	switch (channel)
	{
		case CHANNEL_AW:	AWVALID = value; break;
		case CHANNEL_W:		WVALID = value; break;
		case CHANNEL_B:		BVALID = value; break;
		case CHANNEL_AR:	ARVALID = value; break;
		case CHANNEL_R:		RVALID = value; break;
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return;
	}
}

void AXI_BUS::set_ready(int channel, bool value)
{
	switch (channel)
	{
		case CHANNEL_AW:	AWREADY = value; break;
		case CHANNEL_W:		WREADY = value; break;
		case CHANNEL_B:		BREADY = value; break;
		case CHANNEL_AR:	ARREADY = value; break;
		case CHANNEL_R:		RREADY = value; break;
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return;
	}
}

void AXI_BUS::send_info(int channel, axi_info_t info)
{
	switch(channel)
	{
		case CHANNEL_AW:	AWID = info.id;
							AWADDR = info.addr;
							AWLEN = info.addr_len;
							break;
		case CHANNEL_W:		WID = info.id;
							WDATA = info.data;
							WLAST = info.is_last;
							break;
		case CHANNEL_B:		BID = info.id;
							break;
		case CHANNEL_AR:	ARID = info.id;
							ARADDR = info.addr;
							ARLEN = info.addr_len;
							break;
		case CHANNEL_R:		RID = info.id;
							RDATA = info.data;
							RLAST = info.is_last;
							break;
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return;
	}
}

axi_info_t AXI_BUS::create_null_info()
{
	axi_info_t info;

	info.id = CHANNEL_X;
	info.addr = 0;
	info.addr_len = 0;
	info.data = 0;
	info.is_last = false;
	return info;
}

axi_info_t AXI_BUS::recv_info(int channel)
{
	axi_info_t info;

	info.channel = CHANNEL_X;
	info.id = 0;

	switch(channel)
	{
		case CHANNEL_AW:	info.id = AWID;
							info.addr = AWADDR;
							info.addr_len = AWLEN;
							break;
		case CHANNEL_W:		info.id = WID;
							info.data = WDATA = info.data;
							WLAST = info.is_last;
							break;
		case CHANNEL_B:		info.id = BID;
							break;
		case CHANNEL_AR:	info.id = ARID;
							info.id = ARADDR;
							info.id = ARLEN;
							break;
		case CHANNEL_R:		info.id = RID;
							info.data = RDATA;
							info.is_last = RLAST;
							break;
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
	}
	return (info);
}

void AXI_BUS::channel_receiver(int channel, std::queue<axi_info_t>q)
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (is_ready(channel) == 0)
	{
		log_action = CHANNEL_NOT_READY;
	}
	else if (is_valid(channel))	// ready and valid
	{
		axi_info_t info = recv_info(channel);
		//callback(this, channel, BUS_EVENT_)

	}
}

void AXI_BUS::channel_sender(int channel, std::queue<axi_info_t> q)
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (!q.empty())
	{
		if (is_ready(channel) == 0 && is_valid(channel) == 1)
		{
			// The receiver did not take current data yet.
			// We have to wait until the receiver is ready
			log_action = CHANNEL_WAITR;
		}
		else
		{
			axi_info_t info = q.front();

			if (is_valid(channel))
			{
				log_action = CHANNEL_SENDC;
			}
			else
			{
				log_action = CHANNEL_SEND;
			}

			send_info(channel, info);
			set_valid(channel, true);
			q.pop();

			log_action = CHANNEL_INITIATE;
		}
	}
	else	// Q is empty
	{
		if (is_ready(channel))
		{
			set_valid(channel, false);

			// AXI spec recommends to set zero when not in use
			send_info(channel, create_null_info());
			log_action = CHANNEL_SENDQ;
		}
		else	// empty Q and not ready
		{
			if (is_valid(channel))
			{
				log_action = CHANNEL_WAITR;
			}
			else
			{
				log_action = CHANNEL_IDLE;
			}
		}
	}

	log(channel, log_action, log_detail);

}

void AXI_BUS::log(int channel, std::string log_action, std::string log_detail)
{
	std::string line;
	line = "channel=" + get_channel_name(channel)
		+ log_action + log_detail;
	std::cout << line << std::endl;
}

std::string AXI_BUS::get_channel_name(int channel)
{
	std::string channel_name;

	switch (channel)
	{
		case CHANNEL_AW:	channel_name = "AW";	break;
		case CHANNEL_W:		channel_name = "W";		break;
		case CHANNEL_B:		channel_name = "B";		break;
		case CHANNEL_AR:	channel_name = "AR";	break;
		case CHANNEL_R:		channel_name = "R";		break;
		default:			channel_name = "XXX";	break;
	}
	return channel_name;
}

void AXI_BUS::channel_transaction()
{
	channel_sender(CHANNEL_AW, q_send_AW);
	channel_sender(CHANNEL_W, q_send_W);
	channel_sender(CHANNEL_B, q_send_B);
	channel_sender(CHANNEL_AR, q_send_AR);
	channel_sender(CHANNEL_R, q_send_R);

	channel_receiver(CHANNEL_AW, q_recv_AW);
	channel_receiver(CHANNEL_W, q_recv_W);
	channel_receiver(CHANNEL_B, q_recv_B);
	channel_receiver(CHANNEL_AR, q_recv_AR);
	channel_receiver(CHANNEL_R, q_recv_R);
}

void AXI_BUS::fifo_transaction()
{
	fifo_transaction_in_M();
	fifo_transaction_out_M();
	fifo_transaction_in_S();
	fifo_transaction_out_S();
}

void AXI_BUS::fifo_transaction_in_M()
{
	axi_info_t info;
	bool has_data;

	has_data = fifo_in_M.nb_read(info);
	if (!has_data)
	{
		return;
	}

	if (info.is_write)
	{
		q_send_AW.push(info);
		q_send_W.push(info);
	}
	else
	{
		q_send_AR.push(info);
	}
}

void AXI_BUS::fifo_transaction_in_S()
{
	axi_info_t info;
	bool has_data;

	has_data = fifo_in_S.nb_read(info);
	if (has_data)
	{
		return;
	}

	if (info.is_write)
	{
		q_send_B.push(info);
	}
	else
	{
		q_send_R.push(info);
	}
}

void AXI_BUS::fifo_transaction_out_M()
{
	axi_info_t info;

	if (!q_recv_B.empty())
	{
		info = q_recv_B.front();
		//TODO: fill address
		fifo_out_M.nb_write(info);
		q_recv_B.pop();
	}

	if (!q_recv_R.empty())
	{
		info = q_recv_R.front();
		//TODO: fill address
		fifo_out_M.nb_write(info);
		q_recv_R.pop();
	}
}

void AXI_BUS::fifo_transaction_out_S()
{
	axi_info_t info;

	if (!q_recv_AW.empty())
	{
		info = q_recv_AW.front();
		//TODO: fill address
		fifo_out_S.nb_write(info);
		q_recv_AW.pop();
	}

	if (!q_recv_W.empty())
	{
		info = q_recv_W.front();
		//TODO: fill address
		fifo_out_S.nb_write(info);
		q_recv_W.pop();
	}

	if (!q_recv_AR.empty())
	{
		info = q_recv_AR.front();
		//TODO: fill address
		fifo_out_S.nb_write(info);
		q_recv_AR.pop();
	}
}

uint32_t AXI_BUS::generate_transaction_id()
{
	static uint32_t id = 0;
	id ++;
	return id;
}

/*
int AXI_BUS::callback_default(AXI_BUS* bus, int channel, int event, axi_info_t info)
{
	std::string log_detail = "";
	log_detail = "CALLBACK: channel=" + std::to_string(channel)
			+ ", event=" + std::to_string(event)
			+ "";
	std::cout << log_detail << std::endl;
}
	*/