SC_MODULE(SEQUENCE_DETECTOR)
{
	// input ports
	sc_in<bool>	clock;
	sc_in<bool>	reset;
	sc_in<uint8_t> in;

	// output ports
	sc_out<bool> out;

	// internal data
	uint8_t detect[4];
	uint8_t x0, x1, x2, x3;

	SC_CTOR(SEQUENCE_DETECTOR)
	{
		SC_THREAD(thread_execute);
		sensitive << reset << clock.pos();
	}

	void on_reset()
	{
		detect[3] = 2;
		detect[2] = 0;
		detect[1] = 2;
		detect[0] = 5;

		x3 = 0;
		x2 = 0;
		x1 = 0;
		x0 = 0;
		std::cout << sc_time_stamp() << ": on_reset" << std::endl;
	}

	void on_clock()
	{
		std::cout << sc_time_stamp() << ": on_clock" << std::endl;
		x3 = x2;
		x2 = x1;
		x1 = x0;
		x0 = in;

		if ((x3 == detect [3]) &&
			(x2 == detect [2]) &&
			(x1 == detect [1]) &&
			(x0 == detect [0]))
		{
			std::cout << sc_time_stamp() << ": sequence match" << std::endl;
			out = true;
		}
		else
		{
			std::cout << sc_time_stamp() << ":  " << (int) x3 << "/" << (int) x2 << "/" << (int) x1 << "/" << (int) x0 << std::endl;
			out = false;
		}
	}

	void thread_execute()
	{
		while(true)
		{
			if (reset == true)
			{
				on_reset();
			}
			else if (clock.posedge())
			{
				on_clock();
			}
			wait();
		}
		std::cout << sc_time_stamp() << ": execute end" << std::endl;
	}
};
