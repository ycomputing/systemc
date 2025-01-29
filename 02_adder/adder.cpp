#include <iostream>
#include <systemc>

using namespace sc_core;

#define SIMULATION_TIME	100

SC_MODULE(ADDER)
{
	// input ports
	sc_in<int32_t>	a;
	sc_in<int32_t>	b;

	// output ports
	sc_out<int32_t>	c;

	SC_CTOR(ADDER)
	{
		SC_THREAD(thread_execute);
		sensitive << a << b;
	}

	void thread_execute()
	{
		std::cout << sc_time_stamp() << ": execute begin" << std::endl;
		while(true)
		{
			std::cout << sc_time_stamp() << ": add " << a << " + " << b << std::endl;

			c = a + b;
			wait();
		}
		std::cout << sc_time_stamp() << ": execute end" << std::endl;
	}
};

int sc_main(int, char*[])
{
	sc_signal <int32_t> x;
	sc_signal <int32_t> y;
	sc_signal <int32_t> z;

	ADDER add1("adder1");

	add1.a(x);
	add1.b(y);
	add1.c(z);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	f->set_time_unit(1, SC_NS);
	sc_trace(f, add1.a, "A");
	sc_trace(f, add1.b, "B");
	sc_trace(f, add1.c, "C");
	sc_start(SIMULATION_TIME, SC_NS);
	sc_close_vcd_trace_file(f);
	return (0);
}


