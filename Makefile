threads=4
host=pp5,pp6
graph=graphs/grid100x100.graph
ifdef PPWS
	std=c++14
else
	std=c++23
endif

all:
	g++ -I ./common main.cpp serial.cpp thread.cpp mp.cpp common/graph.cpp common/utils.cpp \
	-o pp-final \
	-std=$(std) \
	-O3 \
	-lpthread \
	-fopenmp

clean:
	rm -f pp-final gen

gen:
	g++ -I ./common gen.cpp common/graph.cpp -o gen -std=$(std) -O3 -lpthread

mpi:
	mpic++ -I ./common serial.cpp mpi.cpp common/graph.cpp common/utils.cpp \
	-o pp-final-mpi \
	-std=$(std) \
	-O3 \
	-lpthread \
	-fopenmp
	$(MAKE) copy

run-mpi:
	mpirun --bind-to none -host $(host) ./pp-final-mpi $(graph) $(threads)

copy:
	parallel-scp -h hosts -r ~/shortest-path ~
	g++ -I ./common main.cpp serial.cpp thread.cpp mp.cpp common/graph.cpp -o pp-final -std=c++23 -O3 -lpthread -fopenmp

