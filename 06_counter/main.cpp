#include <iostream>
#include <systemc>

using namespace sc_core;

#define SIMULATION_TIME	100

#include "counter.h"
#include "resetter.h"

int sc_main(int, char*[])
{
	sc_signal<bool> reset;
	sc_signal<uint32_t> count;

	sc_clock clock ("clock", 2, SC_NS);
	RESETTER r1("resetter");
	COUNTER c1("counter");

	c1.clock(clock);
	c1.reset(reset);
	c1.count(count);

	r1.reset(reset);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, clock, "clock");
	sc_trace(f, reset, "reset");
	sc_trace(f, count, "count");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}
