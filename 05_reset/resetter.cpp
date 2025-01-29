#include <iostream>
#include <systemc>

using namespace sc_core;

#define SIMULATION_TIME	100

SC_MODULE(RESETTER)
{
	// output ports
	sc_out<bool>	reset;

	SC_CTOR(RESETTER)
	{
		SC_THREAD(thread_execute);
	}

	void thread_execute()
	{
		reset = false;
		wait(13, SC_NS);
		reset = true;
		SC_REPORT_INFO ("reset", "example info message, can continue");
		SC_REPORT_WARNING ("reset", "example warning message, can continue");
		//SC_REPORT_ERROR ("reset", "example error message, throws an exception");
		//SC_REPORT_FATAL ("reset", "example fatal message, aborts and may dump core");
		wait(20, SC_NS);
		reset = false;
		wait(40, SC_NS);
		reset = true;
		SC_REPORT_INFO ("reset", "Can this reach here?");
		wait(10, SC_NS);
		reset = false;
	}
};

int sc_main(int, char*[])
{
	sc_signal<bool> x;
	RESETTER r("reset");

	r.reset(x);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, x, "x");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}
