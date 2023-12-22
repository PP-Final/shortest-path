#include <cstring>
#include <chrono>
#include <cstdio>
#include <climits>

#include <iostream>
#include <vector>

#include "common/graph.h"
#include "impl.h"

void output_result(const std::vector<std::vector<int>> &ans, const int n) {
    // table
    std::cout << "    ";
    for (int i = 0; i < n; i++) {
        std::cout << i << "\t";
    }
    std::cout << "\n";
    for (int i = 0; i < n; i++) {
        std::cout << i << "   ";
        for (int j = 0; j < n; j++) {
            if (ans[i][j] == INT_MAX) {
                std::cout << "inf\t";
            } else {
                std::cout << ans[i][j] << "\t";
            }
        }
        std::cout << "\n";
    }
}

template <class T1, class T2>
void output_summary(T1 ref, T2 impl){
    std::printf(
        "%ld ms (%fx speedup)\n",
        std::chrono::duration_cast<std::chrono::microseconds>(impl).count(),
        ref / impl
    );
}

int main(int argc, char** argv) {
    // Check arguments
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " [method] [path] [num_threads]\n";
        return 1;
    }

    // Initialize timer
    std::chrono::high_resolution_clock sc;

    // Load graph
    Graph g = load_graph_binary(argv[2]);
    const std::size_t n = g->num_nodes;

    //print_graph(g);

    // Calculate ref answer
    std::vector<std::vector<int>> ref_ans(n, std::vector<int>(n, INT_MAX));
    std::cout << "Calculating ref answer...\n";
    auto ref_start = sc.now(); 
    dijk_serial(g, ref_ans, n);
    auto ref_end = sc.now();
    auto ref_time_span = static_cast<std::chrono::duration<double>>(ref_end - ref_start);
    std::cout << "ref answer done.\n";

    // Calculate answer with given method
    std::vector<std::vector<int>> ans(n, std::vector<int>(n, INT_MAX));
    std::cout << "Calculating answer...\n";
    auto start, end;
    if (strncmp(argv[1], "serial", 6) == 0) {
        start = sc.now(); 
        dijk_serial(g, ans, n);
        end = sc.now();
    }
    else if (strncmp(argv[1], "thread", 6) == 0) {
        start = sc.now(); 
        dijk_thread(g, ans, n);
        end = sc.now();
    }
    else if (strncmp(argv[1], "mp", 2) == 0) {
        start = sc.now();
        std::cerr << "Not implemented yet.\n";
        end = sc.now();
        return 2;
    }
    else if (strncmp(argv[1], "mpi", 3) == 0) {
        start = sc.now();
        std::cerr << "Not implemented yet.\n";
        end = sc.now();
        return 2;
    }
    else if (strncmp(argv[1], "cuda", 4) == 0) {
        start = sc.now();
        std::cerr << "Not implemented yet.\n";
        end = sc.now();
        return 2;
    }
    else {
        std::cerr << "Invalid method: " << argv[1] << "\n";
        return 1;
    }
    auto time_span = static_cast<std::chrono::duration<double>>(end - start);
    std::cout << "answer calc done.\n";

    // Output result
    output_summary(ref_time_span, time_span);
    std::cout << std::endl;
    return 0;
}
