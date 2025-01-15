#include <iostream>
#include <systemc>

#include <cstdlib>
#include <ctime>

using namespace sc_core;

#define SIMULATION_TIME	100

#include "genrandom.h"

int sc_main(int, char*[])
{
	srand(time(NULL));

	sc_signal <int32_t> x;

	GENRANDOM rand1("random1");

	rand1.ooo(x);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, x, "x");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}


