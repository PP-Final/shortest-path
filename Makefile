threads=4
host=pp5,pp6
graph=graphs/grid100x100.graph

all:
	g++ -I ./common main.cpp serial.cpp thread.cpp mp.cpp common/graph.cpp common/utils.cpp \
	-o pp-final \
	-std=c++23 \
	-O3 \
	-lpthread \
	-fopenmp

mpi:
	mpic++ -I ./common serial.cpp mpi.cpp common/graph.cpp common/utils.cpp \
	-o pp-final-mpi \
	-O3 \
	-lpthread \
	-std=c++23 \
	-fopenmp
	$(MAKE) copy

run-mpi:
	mpirun --bind-to none -host $(host) ./pp-final-mpi $(graph) $(threads)

copy:
	parallel-scp -h hosts -r ~/shortest-path ~
