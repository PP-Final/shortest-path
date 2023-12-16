all:
	g++ -I ./common main.cpp serial.cpp common/graph.cpp -o pp-final -std=c++11 -lpthread 