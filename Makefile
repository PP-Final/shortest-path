all:
	g++ -I ./common main.cpp serial.cpp thread.cpp mp.cpp common/graph.cpp -o pp-final -std=c++23 -O3 -lpthread -fopenmp
