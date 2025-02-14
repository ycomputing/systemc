#!/usr/bin/env python3
import random

def gen_random_access(filename="random_access.csv", address_width=64, data_width=8*64, mode="R",
					  stamp_start=100, stamp_step_min = 10, stamp_step_max = 100,
					  n=100):
	address_mask = (2**(address_width) - 1) ^ ((data_width//8)-1)
	random.seed(0)  # for reproducibility
	f=open(filename, "w")
	stamp = stamp_start
	for i in range(n):
		stamp += random.randrange(10, 100)
		address_bits = random.getrandbits(address_width)
		address_bits_masked = address_bits & address_mask
		address_str = "{:0{}x}".format(address_bits_masked, address_width//4)
		data_bits = random.getrandbits(data_width)
		data_str = "{:0{}x}".format(data_bits, data_width//4)
		read_or_write = mode
		if (mode != "R") and (mode != "W"):
			read_or_write = random.choice(["R", "W"])
		if read_or_write == "R":
			data_str = "0"*(data_width//4)
		f.write("{},{},{},{}\n".format(stamp, read_or_write, address_str, data_str))
	f.close()

if __name__ == '__main__':
	gen_random_access(mode="any", data_width=128)
