#!/usr/bin/env python3
import random

def gen_random_access(filename="random_access.csv", address_bits=16, n=1000):
	random.seed(0)  # for reproducibility
	f=open(filename, "w")
	for i in range(n):
		address = random.randint(0, 2 ** address_bits - 1)
		f.write("{},{}\n".format(i* 10 ,address))
	f.close()

if __name__ == '__main__':
	gen_random_access()
