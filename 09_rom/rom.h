#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#define NANO_PER_SEC 1e9

#define ADDRESS_BITS 16

SC_MODULE(ROM)
{
	// input ports
	sc_in<bool>		clock;
	sc_in<uint16_t>	address;

	// output ports
	// 8-bit data bus. You can change the data width to 16, 32, 64, etc.
	sc_out<uint8_t>	data;
	uint8_t* array;

	const char *filename = "memory.csv";

	SC_CTOR(ROM)
	{
		SC_THREAD(thread_execute);
		sensitive << clock.pos();
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
		read_csv();
		while(true)
		{
			if (clock.posedge())
			{
				std::cout << sc_time_stamp() << ": ROM: Reading address " << address << " with data " << (uint64_t) array[address] << std::endl;
				data = array[address];
			}
			wait();
		}
	}
};
