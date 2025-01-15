#include <iostream>
#include <systemc>

#include <cstdlib>
#include <ctime>

using namespace sc_core;

#define SIMULATION_TIME	100

#include "genrandom.h"
#include "adder.h"

int sc_main(int, char*[])
{
	srand(time(NULL));

	sc_signal <int32_t> x;
	sc_signal <int32_t> y;
	sc_signal <int32_t> z;

	GENRANDOM rand1("random1");
	GENRANDOM rand2("random2");
	ADDER add1("adder1");

	rand1.ooo(x);
	rand2.ooo(y);

	add1.a(x);
	add1.b(y);
	add1.c(z);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, x, "x");
	sc_trace(f, y, "y");
	sc_trace(f, z, "z");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}


