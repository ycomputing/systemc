SC_MODULE(COUNTER)
{
	// input ports
	sc_in<bool>	clock;
	sc_in<bool>	reset;

	// output ports
	sc_out<uint32_t>	count;

	SC_CTOR(COUNTER)
	{
		SC_CTHREAD(thread_execute, clock);
		//dont_initialize();
		async_reset_signal_is(reset, true);
	}

	void thread_execute()
	{
		// reset logic up to the first wait
		std::cout << sc_time_stamp() << ": reset" << std::endl;
		count.write(0);
		std::cout << sc_time_stamp() << ": reset end" << std::endl;
		wait();

		while(true)	// clock.posedge())
		{
			std::cout << sc_time_stamp() << ": c was " << count << std::endl;
			count.write(count.read() + 1);
			wait();
		}
		std::cout << sc_time_stamp() << ": execute end" << std::endl;
	}
};

