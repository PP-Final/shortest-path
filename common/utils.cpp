#include <iostream>
#include <climits>
#include <chrono>
#include "graph.h"
#include "utils.h"

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
            if (ans[i][j] == SHRT_MAX) {
                std::cout << "inf\t";
            } else {
                std::cout << ans[i][j] << "\t";
            }
        }
        std::cout << "\n";
    }
}

template <class T1, class T2>
void output_summary(T1 ref, T2 impl) {
    std::printf(
        "%ld ms (%fx speedup)\n",
        std::chrono::duration_cast<std::chrono::microseconds>(impl).count(),
        ref / impl
    );
}

template <>
void output_summary(std::chrono::duration<double> ref, std::chrono::duration<double> impl) {
    std::printf(
        "%lf s (%fx speedup)\n",
        impl.count(),
        ref / impl
    );
}

Answer init_ans(size_t n){
    Answer ans = new Distance*[n];
    for (int i = 0; i < n; i++) {
        ans[i] = new Distance[n];
        for (int j = 0; j < n; j++) {
            ans[i][j] = SHRT_MAX;
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