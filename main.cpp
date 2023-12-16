#include <iostream>
#include <climits>
#include <vector>
#include <cstring>
#include "common/graph.h"
#include "impl.h"

using namespace std;

void output_result(const vector<vector<int>> &ans, const int n) {
    // table
    cout << "    ";
    for (int i = 0; i < n; i++) {
        cout << i << "\t";
    }
    cout << "\n";
    for (int i = 0; i < n; i++) {
        cout << i << "   ";
        for (int j = 0; j < n; j++) {
            if (ans[i][j] == INT_MAX) {
                cout << "inf\t";
            } else {
                cout << ans[i][j] << "\t";
            }
        }
        cout << "\n";
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Usage: ./pp-final [method] [path]\n";
        return 1;
    }
    Graph g = load_graph_binary(argv[2]);
    const int n = g->num_nodes;
    if (strcmp(argv[1], "serial") == 0) {
        cout << "[Running serial version]\n";
        vector<vector<int>> ans(n, vector<int>(n));
        dijk_serial(g, ans, n);
        output_result(ans, n);
    }
}
