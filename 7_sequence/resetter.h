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
		wait(20, SC_NS);
		reset = false;
		wait(40, SC_NS);
		reset = true;
		wait(10, SC_NS);
		reset = false;
	}
};
