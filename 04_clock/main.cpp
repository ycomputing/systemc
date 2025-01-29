#include <iostream>
#include <systemc>

using namespace sc_core;

#define SIMULATION_TIME	100

int sc_main(int, char*[])
{
	sc_clock clk ("clk", 10, SC_NS);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, clk, "clk");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}


