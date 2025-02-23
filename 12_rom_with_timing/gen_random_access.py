#!/usr/bin/env python3
import random

def gen_random_access(filename="random_access.csv", address_bits=16, n=1000):
	random.seed(0)  # for reproducibility
	f=open(filename, "w")
	address = random.randint(0, 2 ** address_bits - 1)
	ce_bar = random.choices([0, 1], [0.7, 0.3])[0]
	for i in range(n):
		shall_change_address = random.choices([False, True], [0.9, 0.1])[0]
		if shall_change_address:
			address = random.randint(0, 2 ** address_bits - 1)

		shall_change_ce_bar = random.choices([False, True], [0.9, 0.1])[0]
		if shall_change_ce_bar:
			if ce_bar == 1:
				ce_bar = 0
			else:
				ce_bar = 1
		f.write("{},{},{}\n".format(i* 291 ,address, ce_bar))
	f.close()

if __name__ == '__main__':
	gen_random_access()
