#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#define NANO_PER_SEC 1e9

#define ADDRESS_BITS 16
#define DATA_BITS 8

SC_MODULE(ROM)
{
	// input ports
	sc_in<sc_lv<ADDRESS_BITS>> address;
	sc_in<sc_logic>	ce_bar;

	// output ports
	// 8-bit data bus. You can change the data width to 16, 32, 64, etc.
	sc_out<sc_lv<DATA_BITS>>	data;

	sc_lv<DATA_BITS> data_internal;

	uint8_t* array;
	const char *filename = "memory.csv";
	sc_event e_out;

	// based on the timing diagram of Intel 2716 EPROM

	const sc_time T_ACC = sc_time(450, SC_NS);		// from address to output
	const sc_time T_CE = sc_time(450, SC_NS);	// from ce_bar to output
	const sc_time T_DF = sc_time(100, SC_NS);	// from ce_bar to output float
	const sc_time T_OH = sc_time(1, SC_NS);	// output hold

	SC_CTOR(ROM)
	{
		SC_THREAD(thread_execute);
		sensitive << ce_bar << address << e_out;
	}

	void read_csv()
	{
		int array_size = (int) pow(2, ADDRESS_BITS);
		array = new uint8_t [array_size];
		std::ifstream f(filename);
		if (!f.is_open())
		{
			std::cerr << "Error: could not open " << filename << std::endl;
			return;
		}

		int line_number = 0;
		std::string line;
		while (std::getline(f, line))
		{
			std::istringstream iss(line);
			u_int64_t address_value, data_value;
			char comma;
			if (!(iss >> address_value >> comma >> data_value) || comma != ',')
			{
				std::cerr << "Error: invalid format in " << filename << std::endl;
				std::cerr << "At line (" << line_number << "): " << line << std::endl;
				continue;
			}
			array[address_value] = data_value;
			line_number ++;
		}
	}

	void thread_execute()
	{
		sc_time last_ce_posedge = SC_ZERO_TIME;
		sc_time last_ce_negedge = SC_ZERO_TIME;
		sc_time last_address_change = SC_ZERO_TIME;

		read_csv();
		while(true)
		{
			if (ce_bar.posedge())
			{
				last_ce_posedge = sc_time_stamp();
				e_out.notify(T_OH);
				e_out.notify(T_DF);
			}
			else if (ce_bar.negedge())
			{
				last_ce_negedge = sc_time_stamp();
				e_out.notify(T_CE);
			}
			else if (address.event())
			{
				last_address_change = sc_time_stamp();
				e_out.notify(T_ACC);
			}

			if (sc_time_stamp() > last_address_change + T_ACC)
			{
				uint16_t addr = address.read().to_uint();
				data_internal = array[addr];
			}
			else
			{
				data_internal = "XXXXXXXX";
			}

			if (ce_bar == SC_LOGIC_0)
			{
				if (sc_time_stamp() > last_ce_negedge + T_CE)
				{
					data = data_internal;
				}
				else
				{
					data = "ZZZZZZZZ";
				}
			}
			else	// ce_bar == 1
			{
				if (sc_time_stamp() < last_ce_posedge + T_OH)
				{
					data = data_internal;
				}
				else if (sc_time_stamp() < last_ce_posedge + T_DF)
				{
					data = "XXXXXXXX";
				}
				else
				{
					data = "ZZZZZZZZ";
				}
			}
			
			wait();
		}
	}

};
