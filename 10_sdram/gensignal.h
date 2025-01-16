#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#define NANO_PER_SEC 1e9

SC_MODULE(GENSIGNAL)
{
	// output ports
	sc_out<uint16_t>	address;
	const char *filename = "random_access.csv";
	std::vector< std::vector<uint64_t> > array;

	SC_CTOR(GENSIGNAL)
	{
		SC_THREAD(thread_execute);
	}

	void read_csv()
	{
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
			std::vector<uint64_t> row;
			std::istringstream iss(line);
			u_int64_t clock, address;
			char comma;
			if (!(iss >> clock >> comma >> address) || comma != ',')
			{
				std::cerr << "Error: invalid format in " << filename << std::endl;
				std::cerr << "At line (" << line_number << "): " << line << std::endl;
				continue;
			}
			row.push_back(clock);
			row.push_back(address);
			array.push_back(row);
			line_number ++;
		}
	}

	void thread_execute()
	{
		read_csv();
		for (auto row : array)
		{
			sc_time signal_at = sc_time(row[0], SC_NS);
			if (signal_at < sc_time_stamp())
			{
				std::cout << "skip signal=" << signal_at.value() << "stamp=" << sc_time_stamp().value() << std::endl;
				continue;
			}
			else
			{
				double nano_to_wait = (signal_at.to_seconds() - sc_time_stamp().to_seconds()) * NANO_PER_SEC;
				//std::cout << sc_time_stamp() << ": waiting " << nano_to_wait << " ns" << std::endl;
				wait(nano_to_wait, SC_NS);
				address = row[1];
				//std::cout << sc_time_stamp() << ": put digit " << row[1] << std::endl;
			}
		}
	}
};
