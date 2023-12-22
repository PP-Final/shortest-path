#include <vector>
#include <climits>
#include <thread>
#include <mutex>
#include <queue>
#include "common/graph.h"
#include "impl.h"

void dijk_thread_sub(Graph graph, Answer ans, const int n, int thread_id, int num_threads) {
    for (int i = thread_id; i < n; i += num_threads) {
        ans[i][i] = 0;
        std::vector<bool> visited(n, false);

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
            int o_size = outgoing_size(graph, u);

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
}

void dijk_thread(Graph graph, Answer ans, const int n) {
    const auto num_threads = std::thread::hardware_concurrency();
    std::vector<std::jthread> threads;

    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(dijk_thread_sub, graph, ans, n, i, num_threads);
    }
}
