#!/usr/bin/env python3
import random

def gen_random_memory(filename="memory.csv", address_bits=16, data_bits=8):
	random.seed(0)  # for reproducibility
	f = open(filename, "w")
	for address in range(2 ** address_bits):
		data = random.randint(0, 2 ** data_bits - 1)
		f.write("{},{}\n".format(address, data))
	f.close()

if __name__ == '__main__':
	gen_random_memory()
