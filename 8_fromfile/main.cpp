#include <iostream>
#include <systemc>

using namespace sc_core;

#define SIMULATION_TIME	100000

#include "resetter.h"
#include "gensignal.h"
#include "sequence_detector.h"

int sc_main(int, char*[])
{
	sc_signal<bool> reset;
	sc_signal<uint8_t> random_digit;
	sc_signal<bool> match;

	sc_clock clock ("clock", 4, SC_NS);
	RESETTER r1("resetter");
	SEQUENCE_DETECTOR sd1("detector");
	GENSIGNAL gen1("gen");

	sd1.clock(clock);
	sd1.reset(reset);
	sd1.in(random_digit);
	sd1.out(match);

	gen1.digit(random_digit);

	r1.reset(reset);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, clock, "clock");
	sc_trace(f, reset, "reset");
	sc_trace(f, random_digit, "rand");
	sc_trace(f, match, "match");
	sc_trace(f, sd1.x3, "3");
	sc_trace(f, sd1.x2, "2");
	sc_trace(f, sd1.x1, "1");
	sc_trace(f, sd1.x0, "0");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}
