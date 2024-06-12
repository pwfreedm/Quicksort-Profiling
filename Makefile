CC=g++
CFLAGS = -o3
DEPS = Timer.hpp
OBJ = process.o

process: Controller.cpp
	g++ -o QuickSorts Controller.cpp -O3 -std=c++20