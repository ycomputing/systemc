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
			period = rand() % 5;
			std::cout << sc_time_stamp() << ", value=" << value << ", period=" << period << std::endl;

			ooo = value;
			wait(period, SC_NS);
		}
		std::cout << sc_time_stamp() << ": execute end" << std::endl;
	}
};
