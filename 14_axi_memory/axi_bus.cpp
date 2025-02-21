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

// When you (don't) want to see channel signal activity
// (comment out or) uncomment the following line.
//#define DEBUG_AXI_BUS_CHANNEL

// When you want to see progress dump
//#define DEBUG_AXI_BUS_PROGRESS

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
	transaction_request_M();
	transaction_response_S();

	channel_transaction();

	transaction_response_M();
	transaction_request_S();
}

void AXI_BUS::fifo_transaction ()
{
	transaction_request_M();
	transaction_response_S();
	transaction_response_M();
	transaction_request_S();
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

void AXI_BUS::send_info(int channel, axi_bus_info_t& info)
{
	switch(channel)
	{
		case CHANNEL_AW:	AWID = info.id;
							AWADDR = info.addr;
							AWLEN = info.len;
							break;
		case CHANNEL_W:		WID = info.id;
							WDATA = info.data;
							WLAST = info.is_last;
							break;
		case CHANNEL_B:		BID = info.id;
							break;
		case CHANNEL_AR:	ARID = info.id;
							ARADDR = info.addr;
							ARLEN = info.len;
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

axi_bus_info_t AXI_BUS::create_null_info()
{
	axi_bus_info_t info;

	info.id = CHANNEL_X;
	info.addr = 0;
	info.len = 0;
	info.data = 0;
	info.is_last = false;
	return info;
}

axi_bus_info_t AXI_BUS::recv_info(int channel)
{
	axi_bus_info_t info = create_null_info();

	info.channel = CHANNEL_X;
	info.id = 0;

	switch(channel)
	{
		case CHANNEL_AW:	info.id = AWID;
							info.addr = AWADDR;
							info.len = AWLEN;
							break;
		case CHANNEL_W:		info.id = WID;
							info.data = WDATA;
							info.is_last = WLAST;
							break;
		case CHANNEL_B:		info.id = BID;
							break;
		case CHANNEL_AR:	info.id = ARID;
							info.addr = ARADDR;
							info.len = ARLEN;
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

void AXI_BUS::channel_receiver(int channel, std::queue<axi_bus_info_t>& q)
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";

	if (is_ready(channel) == 0)
	{
		set_ready(channel, true);
		log_action = CHANNEL_NOT_READY;
	}
	else if (is_valid(channel))	// ready and valid
	{
		axi_bus_info_t info = recv_info(channel);
		q.push(info);
		log_action = CHANNEL_RECV;
		log_detail = bus_info_to_string(info);
	}
	else	// ready but not valid
	{
		log_action = CHANNEL_WAITV;
	}
	log(channel, log_action, log_detail);
}

void AXI_BUS::channel_sender(int channel, std::queue<axi_bus_info_t>& q)
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";
	axi_bus_info_t info;

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
			info = q.front();

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

			log_action = CHANNEL_SEND;
			log_detail += ", " + bus_info_to_string(info);
		}
	}
	else	// Q is empty
	{
		if (is_ready(channel))
		{
			if (is_valid(channel))
			{
				// was not idle before, now idle.

				set_valid(channel, false);

				// AXI spec recommends to set zero when not in use
				info = create_null_info();
				send_info(channel, info);
				log_action = CHANNEL_IDLE;
			}
			else
			{
				// was idle before, now idle again
				log_action = CHANNEL_IDLE;
			}

		}
		else	// Q is empty and not ready
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

void AXI_BUS::log(int channel, std::string action, std::string detail)
{
#ifdef DEBUG_AXI_BUS_CHANNEL
	log(get_channel_name(channel), action, detail);
#endif
	return;
}

void AXI_BUS::log(std::string source, std::string action, std::string detail)
{
	std::string out;
	out = sc_time_stamp().to_string() + ":" + source
		+ ":" + action + ":" + detail;
	std::cout << out << std::endl;

	return;
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
	channel_receiver(CHANNEL_AW, q_recv_AW);
	channel_receiver(CHANNEL_W, q_recv_W);
	channel_receiver(CHANNEL_B, q_recv_B);
	channel_receiver(CHANNEL_AR, q_recv_AR);
	channel_receiver(CHANNEL_R, q_recv_R);

	channel_sender(CHANNEL_AW, q_send_AW);
	channel_sender(CHANNEL_W, q_send_W);
	channel_sender(CHANNEL_B, q_send_B);
	channel_sender(CHANNEL_AR, q_send_AR);
	channel_sender(CHANNEL_R, q_send_R);
}

void AXI_BUS::transaction_request_M()
{
	axi_trans_t trans;
	bool has_trans;

	has_trans = request_M.nb_read(trans);
	if (!has_trans)
	{
		return;
	}

	uint32_t id = generate_transaction_id();

	axi_bus_info_t info = create_null_info();
	info.id = id;
	info.addr = trans.addr;
	info.len = trans.length - 1;

	if (trans.is_write)
	{
		q_send_AW.push(info);

		info.is_last = false;
		for (int i = 0; i < trans.length; i++)
		{
			if (i == info.len)
			{
				info.is_last = true;
			}
			info.data = trans.data[i];
			q_send_W.push(info);
		}
	}
	else
	{
		progress_create(info, trans.is_write);
		q_send_AR.push(info);
	}
}

void AXI_BUS::transaction_response_S()
{
	axi_trans_t trans;
	bool has_trans;

	has_trans = response_S.nb_read(trans);
	if (!has_trans)
	{
		return;
	}

	uint32_t id;
	tuple_progress_t progress;
	axi_trans_t trans_in_progress;
	bool found = false;

	for (auto iter: map_progress)
	{
		id = iter.first;
		progress = iter.second;
		trans_in_progress = std::get<0>(progress);

		if (trans_in_progress.addr != trans.addr)
		{
			continue;
		}

		found = true;
		break;
	}

	if (found == false)
	{
		log(__FUNCTION__, "Response not in progress",transaction_to_string(trans));
		progress_dump();
		SC_REPORT_FATAL("Response, not in progress", transaction_to_string(trans).c_str());
		return;
	}

	axi_bus_info_t info = create_null_info();
	info.id = id;
	info.addr = trans.addr;
	info.len = trans.length - 1;
	if (trans.is_write)
	{
		q_send_B.push(info);
	}
	else
	{
		info.is_last = false;
		for (int i = 0; i < trans.length; i++)
		{
			if (i == info.len)
			{
				info.is_last = true;
			}
			info.data = trans.data[i];
			q_send_R.push(info);
		}
	}
}

bool AXI_BUS::progress_create(axi_bus_info_t& info, bool is_write)
{
	std::string log_detail;

	axi_trans_t trans;
	auto iter = map_progress.find(info.id);
	if (iter != map_progress.end())
	{
		// Duplicate ID
		log(__FUNCTION__, "DUPLICATE", bus_info_to_string(info));
		progress_dump();
		SC_REPORT_FATAL("DUPLICATE ID", bus_info_to_string(info).c_str());
		return false;
	}

	trans.addr = info.addr;
	trans.length = info.len + 1;
	trans.is_write = is_write;
	map_progress[info.id] = std::make_tuple(trans, 0);
	log_detail = "id=" + std::to_string(info.id) + ", " + transaction_to_string(trans);
	log(__FUNCTION__, "CREATE PROGRESS", log_detail);
	progress_dump();
	return true;
}

void AXI_BUS::progress_delete(axi_bus_info_t& info)
{
	auto iter = map_progress.find(info.id);
	if (iter == map_progress.end())
	{
		// No such ID
		log(__FUNCTION__, "NO ID", bus_info_to_string(info));
		progress_dump();
		SC_REPORT_FATAL("NO ID", bus_info_to_string(info).c_str());
		return;
	}

	log(__FUNCTION__, "DELETE PROGRESS", bus_info_to_string(info));
	progress_dump();

	map_progress.erase(iter);
}


// returns true when 100% progress is made.
// you have to pop the q manually when this returns true.

bool AXI_BUS::progress_update(std::queue<axi_bus_info_t>& q)
{
	std::string log_detail;

	axi_bus_info_t info;
	
	if (q.empty())
	{
		return false;
	}

	info = q.front();
	auto iter = map_progress.find(info.id);
	if (iter == map_progress.end())
	{
		// Nothing in progress for that id
		log(__FUNCTION__, "NO ID", bus_info_to_string(info));
		progress_dump();
		SC_REPORT_FATAL("NOID", "q_recv_X");
	}
	auto& progress = iter->second;
	auto& trans_in_progress = std::get<0>(progress);
	int8_t count_done = std::get<1>(progress);
	if (count_done >= trans_in_progress.length)
	{
		// We got more data than required length
		log(__FUNCTION__, "TOO MUCH DATA", bus_info_to_string(info));
		progress_dump();
		SC_REPORT_FATAL("TOO MUCH DATA", "q_recv_X");
	}

	// be careful to update original data
	trans_in_progress.data[count_done] = info.data;
	std::get<1>(progress) = count_done + 1;

	if (info.is_last)
	{
		if (count_done != trans_in_progress.length - 1)
		{
			// We got last data when there must be more
			log_detail = "done=" + std::to_string(count_done + 1) + "/"
			+ std::to_string(trans_in_progress.length) + ", " + bus_info_to_string(info);
			log(__FUNCTION__, "PREMATURE LAST", log_detail);
			progress_dump();
			SC_REPORT_FATAL("PREMATURE LAST", "q_recv_X");
		}
		// progress is 100%.
		// do not pop, do not erase progress yet.
		log_detail = "done=" + std::to_string(count_done + 1) + "/"
			+ std::to_string(trans_in_progress.length) + ", " + bus_info_to_string(info);
		log(__FUNCTION__, "GET FULL", log_detail);
		return true;

	}
	else	// not the last data
	{
		q.pop();
		log_detail = "done=" + std::to_string(count_done + 1) + "/"
			+ std::to_string(trans_in_progress.length) + ", " + bus_info_to_string(info);
		log(__FUNCTION__, "PLUS ONE", log_detail);
	}

	return false;
}

bool AXI_BUS::transaction_send_q(sc_fifo_out<axi_trans_t>& fifo_out, std::queue<axi_bus_info_t>& q)
{
	bool is_accepted;
	std::string log_detail;

	if (q.empty())
	{
		return false;
	}

	axi_bus_info_t& info = q.front();
	auto iter = map_progress.find(info.id);
	if (iter == map_progress.end())
	{
		// Nothing in progress for that id
		log_detail += ", " + bus_info_to_string(info);
		log(__FUNCTION__, "NO ID in progress", log_detail);
		progress_dump();
		SC_REPORT_FATAL("NOID", "q_recv_X");
	}

	auto& progress = iter->second;
	auto& trans_in_progress = std::get<0>(progress);
	is_accepted = fifo_out.nb_write(trans_in_progress);
	if (is_accepted)
	{
		log_detail = "name=";
		log_detail += fifo_out.name();
		log_detail += ", " + transaction_to_string(trans_in_progress);
		log(__FUNCTION__, "SENT TRANS TO FIFO", log_detail);
		return true;
	}
	else
	{
		// Fifo is full, try again in the future.
		log(__FUNCTION__, "FIFO FULL", transaction_to_string(trans_in_progress));
	}
	return false;
}

void AXI_BUS::transaction_response_M()
{
	bool is_accepted;

	// write transaction
	is_accepted = transaction_send_q(response_M, q_recv_B);
	if (is_accepted)
	{
		axi_bus_info_t& info = q_recv_B.front();
		progress_delete(info);
		q_recv_B.pop();
	}

	// read transaction
	bool is_completed = progress_update(q_recv_R);
	if (is_completed)
	{
		is_accepted = transaction_send_q(response_M, q_recv_R);
		if (is_accepted)
		{
			axi_bus_info_t& info = q_recv_R.front();
			progress_delete(info);
			q_recv_R.pop();
		}
	}
}

void AXI_BUS::transaction_request_S()
{
	bool is_accepted;
	if (!q_recv_AW.empty())
	{
		progress_create(q_recv_AW.front(), true);
		q_recv_AW.pop();
	}

	bool is_completed = progress_update(q_recv_W);
	if (is_completed)
	{
		is_accepted = transaction_send_q(request_S, q_recv_W);
		if (is_accepted)
		{
			q_recv_W.pop();
		}
	}

	is_accepted = transaction_send_q(request_S, q_recv_AR);
	if (is_accepted)
	{
		q_recv_AR.pop();
	}
}

uint32_t AXI_BUS::generate_transaction_id()
{
	static uint32_t id = 0;
	id ++;
	return id;
}

std::string AXI_BUS::bus_info_to_string(const axi_bus_info_t& info)
{
	std::string s;
	s = "id=" + std::to_string(info.id)
		+ ", addr=" + address_to_hex_string(info.addr)
		+ ", len=" + std::to_string(info.len)
		+ ", data=" + bus_data_to_hex_string(info.data);
	return s;
}

std::string AXI_BUS::transaction_to_string(const axi_trans_t& trans)
{
	std::string s;
	bool is_first = true;
	s = "addr=" + address_to_hex_string(trans.addr)
		+ ", length=" + std::to_string(trans.length)
		+ ", wr=" + std::to_string(trans.is_write)
		+ ", data=";
	for (int i = 0; i < trans.length; i++)
	{
		if (is_first == false)
		{
			s += ",";
		}
		s += bus_data_to_hex_string(trans.data[i]);
		if (is_first)
		{
			is_first = false;
		}
	}
	return s;
}

std::string AXI_BUS::progress_to_string(const tuple_progress_t& progress)
{
	axi_trans_t trans;
	uint8_t count;
	trans = std::get<0>(progress);
	count = std::get<1>(progress);
	std::string s;
	s = transaction_to_string(trans) + ", progress=" + std::to_string(count);
	return s;
}

void AXI_BUS::progress_dump()
{

#ifdef DEBUG_AXI_BUS_PROGRESS
	//typedef std::tuple<axi_trans_t, int8_t>
	tuple_progress_t progress;
	uint32_t id;
	std::string out;

	out = "----------------------------------------------- progress dump begin\n";
	for (auto iter: map_progress)
	{
		id = iter.first;
		progress = iter.second;
		out += ">>>>id=" + std::to_string(id) + ", " + progress_to_string(progress);
		out += "\n";
	}
	out += "----------------------------------------------- progress dump end\n";

	std::cout << out;
#endif

	return;
}