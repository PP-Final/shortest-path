#include "common/graph.h"
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: ./gen <num_nodes> <num_edges> <filename>\n";
        return 1;
    }

    int num_nodes = std::stoi(argv[1]);
    int num_edges = std::stoi(argv[2]);
    const char* filename = argv[3];
    int num_threads = std::thread::hardware_concurrency();

    if (num_nodes < 0 || num_edges < 0) {
        std::cout << "Invalid arguments\n";
        return 1;
    }

    Graph graph = generate_random_graph(num_nodes, num_edges, num_threads);
    store_graph_binary(filename, graph);
    free_graph(graph);
    return 0;
}
