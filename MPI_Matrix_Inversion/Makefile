CC = mpic++
CFLAGS = -std=c++11

all:
	$(CC) test.cpp -o test

run: all
	mpirun -np 2 main

clean:
	del main