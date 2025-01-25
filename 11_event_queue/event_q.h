#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#define NANO_PER_SEC 1e9


SC_MODULE(EVENT_Q)
{
	// input ports
	sc_in<bool>	clock;

	// output ports
	sc_out<sc_logic>	out;

	sc_event_queue event_queue;

	const sc_time T_AA = sc_time(10, SC_NS);
	const sc_time T_BB = sc_time(20, SC_NS);
	const sc_time T_CC = sc_time(25, SC_NS);
	const sc_time T_DD = sc_time(35, SC_NS);
	const sc_time T_EE = sc_time(40, SC_NS);

	SC_CTOR(EVENT_Q)
	{
		SC_THREAD(thread_execute);
		sensitive << clock << event_queue;
	}

	void thread_execute()
	{
		sc_time last_posedge = SC_ZERO_TIME;

		while(true)
		{
			if (clock.posedge())
			{
				last_posedge = sc_time_stamp();
				event_queue.notify(T_AA);
				event_queue.notify(T_BB);
				event_queue.notify(T_CC);
				event_queue.notify(T_DD);
				event_queue.notify(T_EE);
			}

			if (clock == true)
			{
				if (sc_time_stamp() < last_posedge + T_AA)
				{
					out = SC_LOGIC_X;
				}
				else if (sc_time_stamp() < last_posedge + T_BB)
				{
					out = SC_LOGIC_0;
				}
				else if (sc_time_stamp() < last_posedge + T_CC)
				{
					out = SC_LOGIC_X;
				}
				else if (sc_time_stamp() < last_posedge + T_DD)
				{
					out = SC_LOGIC_1;
				}
				else if (sc_time_stamp() < last_posedge + T_EE)
				{
					out = SC_LOGIC_X;
				}
				else
				{
					out = SC_LOGIC_Z;	
				}
			}
			else
			{
				out = SC_LOGIC_Z;
			}
			
			wait();
		}
	}

};
