#include <iostream>
#include <climits>
#include <vector>
#include <cstring>
#include <chrono>
#include <cstdio>
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
    if (argc != 3) {
        std::cout << "Usage: ./pp-final [method] [path]\n";
        return 1;
    }
    std::chrono::high_resolution_clock sc;

    Graph g = load_graph_binary(argv[2]);
    const std::size_t n = g->num_nodes;

    // Calculate ref answer
    std::vector<std::vector<int>> ref_ans(n, std::vector<int>(n, INT_MAX));
    std::cout << "Calculating ref answer...\n";
    auto ref_start = sc.now(); 
    dijk_serial(g, ref_ans, n);
    auto ref_end = sc.now();
    auto ref_time_span = static_cast<std::chrono::duration<double>>(ref_end - ref_start);
    std::cout << "ref answer done.\n";

    std::vector<std::vector<int>> ans(n, std::vector<int>(n, INT_MAX));
    std::cout << "Calculating answer...\n";
    auto start = sc.now(); 
    if (strncmp(argv[1], "serial", 6) == 0) {
        dijk_serial(g, ans, n);
    }
    else if (strncmp(argv[1], "thread", 6) == 0) {
        dijk_thread(g, ans, n);
    }
    else {
        return 1;
    }
    auto end = sc.now();
    auto time_span = static_cast<std::chrono::duration<double>>(end - start);
    std::cout << "answer calc done.\n";
    output_summary(ref_time_span, time_span);

    std::cout << std::endl;
    return 0;
}
