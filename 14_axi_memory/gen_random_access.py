#!/usr/bin/env python3
import random

def gen_random_access(filename_access="m_access.csv", filename_memory="s_memory.csv",
					  address_bus_width=64, data_bus_width=128, mode="R",
					  length_max=1, is_length_variable=False,
					  stamp_start=100, stamp_step_min = 1, stamp_step_max = 100,
					  n=100):
	address_mask = (2**(address_bus_width) - 1) ^ ((data_bus_width//8)-1)
	zero_data_str = "0x" + "0"*(data_bus_width//4)

	random.seed(0)  # for reproducibility
	f_access = open(filename_access, "w")
	f_memory = open(filename_memory, "w")
	stamp = stamp_start
	for i in range(n):
		stamp += random.randint(stamp_step_min, stamp_step_max)

		length = length_max
		if is_length_variable:
			length = random.randint(1, length_max)

		address_bits_base = random.getrandbits(address_bus_width)

		list_address_str = []
		list_data_str = []
		for inc in range(length):
			address_bits = address_bits_base + (inc * (data_bus_width//8))
			address_bits_masked = address_bits & address_mask
			address_str = "0x{:0{}x}".format(address_bits_masked, address_bus_width//4)
			list_address_str.append(address_str)

			data_bits = random.getrandbits(data_bus_width)
			data_str = "0x{:0{}x}".format(data_bits, data_bus_width//4)
			list_data_str.append(data_str)
	
		read_or_write = mode
		if (mode != "R") and (mode != "W"):
			read_or_write = random.choice(["R", "W"])
		if read_or_write == "R":
			for inc in range(length):
				address_str = list_address_str[inc]
				data_str = list_data_str[inc]
				f_memory.write("{},{}\n".format(address_str, data_str))

		for inc in range(length):
			address_str = list_address_str[inc]
			data_str = list_data_str[inc]
			if read_or_write == "R":
				data_str = zero_data_str
			f_access.write("{},{},{},{},{}\n".format(stamp, read_or_write, address_str, length, data_str))
	f_access.close()
	f_memory.close()

if __name__ == '__main__':
#	gen_random_access(mode="any", n=1000)
#	gen_random_access(mode="any", length_max=4, is_length_variable=True, n=1000)
	gen_random_access(mode="any", length_max=100, is_length_variable=True, n=1000)
