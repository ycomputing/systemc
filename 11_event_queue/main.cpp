#include <iostream>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

#define SIMULATION_TIME	500

#include "event_q.h"

int sc_main(int, char*[])
{
	sc_clock clock("clock", 100, SC_NS);
	sc_signal<sc_logic> out;

	EVENT_Q q("q");

	q.clock(clock);
	q.out(out);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, clock, "clock");
	sc_trace(f, out, "out");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}
