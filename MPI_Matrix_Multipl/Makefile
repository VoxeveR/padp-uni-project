CC = mpic++
CFLAGS = -std=c++11

all:
	$(CC) main.cpp -o main 

run: all
	mpirun -np 4 main

clean:
	del main