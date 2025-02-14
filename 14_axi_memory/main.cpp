#include <iostream>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

#define SIMULATION_TIME	100000

#include "axi_manager.h"
#include "axi_subordinate.h"
#include "resetter.h"

int sc_main(int, char*[])
{
	sc_clock ACLK("ACLK", 1, SC_NS);
	sc_signal<bool> ARESETn;
	
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

	// Chapter A2.1.3 write response channel
	sc_signal<bool>			BVALID;
	sc_signal<bool>			BREADY;
	sc_signal<uint32_t>		BID;
	sc_signal<bool>			BRESP;

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

	RESETTER r("r");
	r.ARESETn(ARESETn);

	AXI_MANAGER m("m");
	AXI_SUBORDINATE s("s");

	m.ACLK(ACLK);
	m.ARESETn(ARESETn);
	m.AWVALID(AWVALID);
	m.AWREADY(AWREADY);
	m.AWID(AWID);
	m.AWADDR(AWADDR);
	m.AWLEN(AWLEN);
	m.WVALID(WVALID);
	m.WREADY(WREADY);
	m.WID(WID);
	m.WDATA(WDATA);
	m.BVALID(BVALID);
	m.BREADY(BREADY);
	m.BID(BID);
	m.BRESP(BRESP);
	m.ARVALID(ARVALID);
	m.ARREADY(ARREADY);
	m.ARID(ARID);
	m.ARADDR(ARADDR);
	m.ARLEN(ARLEN);
	m.RVALID(RVALID);
	m.RREADY(RREADY);
	m.RID(RID);
	m.RDATA(RDATA);
	m.RLAST(RLAST);

	s.ACLK(ACLK);
	s.ARESETn(ARESETn);
	s.AWVALID(AWVALID);
	s.AWREADY(AWREADY);
	s.AWID(AWID);
	s.AWADDR(AWADDR);
	s.AWLEN(AWLEN);
	s.WVALID(WVALID);
	s.WREADY(WREADY);
	s.WID(WID);
	s.WDATA(WDATA);
	s.BVALID(BVALID);
	s.BREADY(BREADY);
	s.BID(BID);
	s.BRESP(BRESP);
	s.ARVALID(ARVALID);
	s.ARREADY(ARREADY);
	s.ARID(ARID);
	s.ARADDR(ARADDR);
	s.ARLEN(ARLEN);
	s.RVALID(RVALID);
	s.RID(RID);
	s.RDATA(RDATA);
	s.RREADY(RREADY);
	s.RLAST(RLAST);


	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	sc_trace(f, ARESETn, "ARESETn");
	sc_trace(f, ACLK, "ACLK");
	sc_trace(f, AWVALID, "AWVALID");
	sc_trace(f, AWREADY, "AWREADY");
	sc_trace(f, AWID, "AWID");
	sc_trace(f, AWADDR, "AWADDR");
	sc_trace(f, AWLEN, "AWLEN");
	sc_trace(f, WVALID, "WVALID");
	sc_trace(f, WREADY, "WREADY");
	sc_trace(f, WID, "WID");
	sc_trace(f, WDATA, "WDATA");
	sc_trace(f, BVALID, "BVALID");
	sc_trace(f, BREADY, "BREADY");
	sc_trace(f, BID, "BID");
	sc_trace(f, BRESP, "BRESP");
	sc_trace(f, ARVALID, "ARVALID");
	sc_trace(f, ARREADY, "ARREADY");
	sc_trace(f, ARID, "ARID");
	sc_trace(f, ARADDR, "ARADDR");
	sc_trace(f, ARLEN, "ARLEN");
	sc_trace(f, RVALID, "RVALID");
	sc_trace(f, RREADY, "RREADY");
	sc_trace(f, RID, "RID");
	sc_trace(f, RDATA, "RDATA");
	sc_trace(f, RLAST, "RLAST");

	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}
