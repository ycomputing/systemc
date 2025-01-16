#include <iostream>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

int sc_main(int, char*[])
{
	sc_logic zero = SC_LOGIC_0;
	sc_logic one = SC_LOGIC_1;
	sc_logic x = SC_LOGIC_X;
	sc_logic z = SC_LOGIC_Z;

	std::cout << "zero = " << zero << std::endl;
	std::cout << "one = " << one << std::endl;
	std::cout << "X = " << x << std::endl;
	std::cout << "Z = " << z << std::endl;

	std::cout << "not 0 = " << (~ SC_LOGIC_0) << std::endl;
	std::cout << "not 1 = " << (~ SC_LOGIC_1) << std::endl;
	std::cout << "not X = " << (~ SC_LOGIC_X) << std::endl;
	std::cout << "not Z = " << (~ SC_LOGIC_Z) << std::endl;
	std::cout << "0 and 1 = " << (SC_LOGIC_0 & SC_LOGIC_1) << std::endl;
	std::cout << "0 and X = " << (SC_LOGIC_0 & SC_LOGIC_X) << std::endl;
	std::cout << "0 or  X = " << (SC_LOGIC_0 | SC_LOGIC_X) << std::endl;
	std::cout << "1 or  X = " << (SC_LOGIC_1 | SC_LOGIC_X) << std::endl;
	std::cout << "0 and Z = " << (SC_LOGIC_0 & SC_LOGIC_Z) << std::endl;
	std::cout << "0 or  Z = " << (SC_LOGIC_0 | SC_LOGIC_Z) << std::endl;
	std::cout << "1 or  Z = " << (SC_LOGIC_1 | SC_LOGIC_Z) << std::endl;
	std::cout << "1 and Z = " << (SC_LOGIC_1 & SC_LOGIC_Z) << std::endl;
	std::cout << "X and Z = " << (SC_LOGIC_X & SC_LOGIC_Z) << std::endl;
	std::cout << "X or  Z = " << (SC_LOGIC_X | SC_LOGIC_Z) << std::endl;

	sc_lv<8> a, b, c;
	a = "0b01010X0Z";
	b = "0b11X0Z1ZX";

	c = a & b;
	std::cout << a << " and " << b << " = " << c << std::endl;

	c = a | b;
	std::cout << a << " or  " << b << " = " << c << std::endl;

	return (0);
}
