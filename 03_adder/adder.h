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

