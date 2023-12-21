#include <vector>
#include <climits>
#include <thread>
#include <mutex>
#include "common/graph.h"

std::mutex mtx; // Mutex for synchronization

void dijk_thread_sub(Graph graph, std::vector<std::vector<int>> &ans, const int n, int thread_id, int num_threads) {
    for (int i = thread_id; i < n; i += num_threads) {
        std::vector<int> dist(n, INT_MAX);
        dist[i] = 0;
        std::vector<bool> visited(n, false);

        for (int j = 0; j < n; j++) {
            int u = -1;

            // Find the unvisited node with the smallest distance
            for (int k = 0; k < n; k++) {
                if (!visited[k] && (u == -1 || dist[k] < dist[u])) {
                    u = k;
                }
            }

            visited[u] = true;

            // Relax neighboring nodes
            for (int k = 0; k < outgoing_size(graph, u); k++) {
                int v = outgoing_begin(graph, u)[k];
                if (dist[u] + 1 < dist[v]) {
                    dist[v] = dist[u] + 1;
                }
            }
        }

        // Lock the mutex before modifying the shared ans vector
        std::lock_guard<std::mutex> lock(mtx);
        ans[i] = dist;
    }
}

void dijk_thread(Graph graph, std::vector<std::vector<int>> &ans, const int n) {
    const auto num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(dijk_thread_sub, graph, std::ref(ans), n, i, num_threads);
    }

    for (auto &thread : threads) {
        thread.join();
    }
}
