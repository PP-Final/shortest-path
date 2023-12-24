#include <cstring>
#include <chrono>
#include <cstdio>
#include <climits>
#include <thread>
#include <iostream>
#include <vector>

#include "common/graph.h"
#include "impl.h"

void output_result(const Answer ans, const int n) {
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

Answer init_ans(size_t n){
    Answer ans = new int*[n];
    for (int i = 0; i < n; i++) {
        ans[i] = new int[n];
        for (int j = 0; j < n; j++) {
            ans[i][j] = INT_MAX;
        }
    }
    return ans;
}

void free_ans(Answer ans, size_t n){
    for (int i = 0; i < n; i++) {
        delete[] ans[i];
    }
    delete[] ans;
}

int main(int argc, char** argv) {
    // Check arguments
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " [method] [path] [num_threads]\n";
        return 1;
    }

    int num_threads = std::thread::hardware_concurrency();
    if (argc >= 4) {
        num_threads = std::atoi(argv[3]);
    }

    // Initialize timer
    std::chrono::high_resolution_clock sc;

    // Load graph
    Graph g = load_graph_binary(argv[2]);
    const std::size_t n = g->num_nodes;
    std::cout << "Graph loaded.\n";
    std::cout << "num_nodes: " << n << "\n";

    //print_graph(g);

    // Calculate ref answer
    auto ref_ans = init_ans(n);
    std::cout << "Calculating ref answer...\n";
    auto ref_start = sc.now(); 
    dijk_serial(g, ref_ans, n);
    auto ref_end = sc.now();
    auto ref_time_span = static_cast<std::chrono::duration<double>>(ref_end - ref_start);
    std::cout << "ref answer done.\n";

    // Calculate answer with given method
    auto ans = init_ans(n);
    std::cout << "Calculating answer...\n";
    auto start = sc.now(), end = sc.now();
    if (strncmp(argv[1], "serial", 6) == 0) {
        start = sc.now(); 
        dijk_serial(g, ans, n);
        end = sc.now();
    }
    else if (strncmp(argv[1], "thread", 6) == 0) {
        std::cout << "num_threads: " << num_threads << "\n";
        start = sc.now(); 
        dijk_thread(g, ans, n, num_threads);
        end = sc.now();
    }
    else if (strncmp(argv[1], "mp", 2) == 0) {
        std::cout << "num_threads: " << num_threads << "\n";
        start = sc.now();
        dijk_mp(g, ans, n, num_threads);
        end = sc.now();
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

    // Verify ans
    for (int i = 0; i < n; i++) {
        if (memcmp(ref_ans[i], ans[i], n * sizeof(ans[0][0])) != 0) {
            std::cerr << "\033[1;31mVerification failed.\033[0m\n";
            return 2;
        }
    }
    std::cout << "\033[1;32mVerification passed.\033[0m\n";

    free_ans(ref_ans, n);
    free_ans(ans, n);
    free_graph(g);

    // Output result
    output_summary(ref_time_span, time_span);
    std::cout << std::endl;
    return 0;
}
