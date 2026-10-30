# Simple makefile

CC := g++ -std=c++20 -O3 -fopenmp

.DEFAULT_GOAL := main.cpp

main.cpp:
	mkdir -p bin
	$(CC) -o ./bin/pm ./src/main.cpp
