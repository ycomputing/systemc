#include <iostream>
#include <systemc>

using namespace sc_core;

#define SIMULATION_TIME	10000

#include "gensignal.h"
#include "rom.h"

int sc_main(int, char*[])
{
	sc_signal<uint16_t> address;
	sc_signal<uint8_t> data;
	sc_clock clock ("clock", 2, SC_NS);

	ROM r1("rom");
	GENSIGNAL gen1("gen");

	r1.clock(clock);
	r1.address(address);
	r1.data(data);

	gen1.address(address);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, clock, "clock");
	sc_trace(f, address, "address");
	sc_trace(f, data, "data");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}
