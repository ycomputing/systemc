SC_MODULE(GENRANDOM)
{
	// output ports
	sc_out<int32_t>	ooo;

	SC_CTOR(GENRANDOM)
	{
		SC_THREAD(thread_execute);
	}

	void thread_execute()
	{
		while (true)
		{
			int32_t	value;
			int32_t	period;
			value = rand() % 100;
			period = rand() % 10;
			ooo = value;

			std::cout << sc_time_stamp() << ", value=" << value << ", period=" << period << ", output=" << ooo << std::endl;
			wait(SC_ZERO_TIME);

			std::cout << sc_time_stamp() << ", value=" << value << ", period=" << period << ", output=" << ooo << std::endl;
			wait(period, SC_NS);
		}
		std::cout << sc_time_stamp() << ": execute end" << std::endl;
	}
};
