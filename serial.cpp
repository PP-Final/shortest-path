#include <vector>
#include <climits>
#include "common/graph.h"

using namespace std;

void dijk_serial(Graph graph, vector<vector<int>> &ans, const int n) {
    for (int i = 0; i < n; i++) {
        vector<int> dist(n, INT_MAX);
        dist[i] = 0;
        vector<bool> visited(n, false);
        for (int j = 0; j < n; j++) {
            int u = -1;
            for (int k = 0; k < n; k++) {
                if (!visited[k] && (u == -1 || dist[k] < dist[u])) {
                    u = k;
                }
            }
            visited[u] = true;
            for (int k = 0; k < outgoing_size(graph, u); k++) {
                int v = outgoing_begin(graph, u)[k];
                if (dist[u] + 1 < dist[v]) {
                    dist[v] = dist[u] + 1;
                }
            }
        }
        ans[i] = dist;
    }
}
