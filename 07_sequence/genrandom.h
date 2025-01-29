SC_MODULE(GENRANDOM)
{
	// input ports
	sc_in<bool> clock;

	// output ports
	sc_out<uint8_t>	ooo;

	SC_CTOR(GENRANDOM)
	{
		SC_CTHREAD(thread_execute, clock.pos());
	}

	void thread_execute()
	{
		srand(0);	// fix the seed for reproducibility
		while (true)
		{
			uint8_t	value;
			value = (uint8_t) (rand() % 10);
			ooo = value;
			wait();
		}
	}
};
