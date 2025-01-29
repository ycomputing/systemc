#!/usr/bin/env python3
import random

def gen_random_digit(filename="random_digit.csv", n=100000):
	random.seed(0)  # for reproducibility
	f=open(filename, "w")
	for i in range(n):
		digit = random.randint(0, 9)
		f.write("{},{}\n".format(i,digit))
	f.close()

if __name__ == '__main__':
	gen_random_digit()
