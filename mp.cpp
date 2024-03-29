#include <queue>
#include <vector>
#include <climits>
#include <omp.h>

#include "common/graph.h"
#include "impl.h"

void dijk_mp(Graph graph, Answer ans, const int n, const int num_threads) {
    #pragma omp parallel num_threads(num_threads)
    {
    #pragma omp for schedule(dynamic, 10)
    for (int i = 0; i < n; i++) {
        ans[i][i] = 0;
        std::vector<bool> visited(n, false);

        // Store ans[i] to avoid repeated function calls
        auto& current_ans = ans[i];

        // Use priority queue for efficient min element extraction
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;
        pq.push({0, i});

        while (!pq.empty()) {
            int u = pq.top().second;
            pq.pop();

            if (visited[u]) continue;
            visited[u] = true;

            auto outgoing = outgoing_begin(graph, u);
            auto weight = weight_begin(graph, u);
            auto o_size = outgoing_size(graph, u);

            for (int k = 0; k < o_size; k++) {
                int v = outgoing[k];
                int new_distance = current_ans[u] + weight[k];

                if (new_distance < current_ans[v]) {
                    current_ans[v] = new_distance;
                    pq.push({new_distance, v});
                }
            }
        }
    }
    } // end of parallel
}