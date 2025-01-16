#include <iostream>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

#define SIMULATION_TIME	10000

#include "rom.h"
#include "gensignal.h"

int sc_main(int, char*[])
{
	sc_signal<sc_lv<ADDRESS_BITS>> address;
	sc_signal<sc_lv<DATA_BITS>> data;
	sc_signal<sc_logic> ce_bar;

	ROM r1("rom");
	GENSIGNAL gen1("gen");

	r1.address(address);
	r1.data(data);
	r1.ce_bar(ce_bar);

	gen1.address(address);
	gen1.ce_bar(ce_bar);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, address, "address");
	sc_trace(f, data, "data");
	sc_trace(f, ce_bar, "ce_bar");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}
