#include "mpi.h"
#include <queue>
#include <omp.h>
#include <vector>
#include <chrono>
#include <climits>
#include <thread>
#include <fstream>

#include "common/graph.h"
#include "common/utils.h"
#include "impl.h"

void dijk_mpi(Graph graph, Answer ans, const int n, const int rank, const int size) {
    int num_nodes_per_proc = n / size;
    std::cout << "rank: " << rank << " is responsible for nodes " << rank * num_nodes_per_proc << " to " << (rank + 1) * num_nodes_per_proc << "\n";

    int start = rank * num_nodes_per_proc;
    int end = rank == size - 1 ? n : (rank + 1) * num_nodes_per_proc;
    for (int i = start; i < end; i++) {
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

    if (rank != 0) {
        // Worker
        for (int i = start; i < end; i++) {
            MPI_Send(ans[i], n, MPI_SHORT, 0, 0, MPI_COMM_WORLD);
        }
    } else {
        // Master
        for (int i = 1; i < size; i++) {
            int start = i * num_nodes_per_proc;
            int end = i == size - 1 ? n : (i + 1) * num_nodes_per_proc;
            for (int j = start; j < end; j++) {
                MPI_Recv(ans[j], n, MPI_SHORT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }

    MPI_Finalize();
}

void dijk_mpi_omp(Graph graph, Answer ans, const int n, const int num_threads, const int rank, const int size) {
    int num_nodes_per_proc = n / size;

    int start = rank * num_nodes_per_proc;
    int end = rank == size - 1 ? n : (rank + 1) * num_nodes_per_proc;

    int cnt[num_threads] = {0};
    #pragma omp parallel num_threads(num_threads)
    {
    #pragma omp for schedule(dynamic, 10)
    for (int i = start; i < end; i++) {
        cnt[omp_get_thread_num()]++;
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

    for (int i = 0; i < num_threads; i++) {
        std::cout << "rank: " << rank << " thread: " << i << " cnt: " << cnt[i] << "\n";
    }

    if (rank != 0) {
        // Worker
        for (int i = start; i < end; i++) {
            MPI_Send(ans[i], n, MPI_SHORT, 0, 0, MPI_COMM_WORLD);
        }
    } else {
        // Master
        for (int i = 1; i < size; i++) {
            int start = i * num_nodes_per_proc;
            int end = i == size - 1 ? n : (i + 1) * num_nodes_per_proc;
            for (int j = start; j < end; j++) {
                MPI_Recv(ans[j], n, MPI_SHORT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }

    MPI_Finalize();
}

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        std::cerr << "Usage: " << argv[0] << " [path] [num_threads]\n";
        return 1;
    }
    int num_threads = 4;
    if (argc == 3) {
        num_threads = std::stoi(argv[2]);
    }

    std::chrono::high_resolution_clock sc;

    if (num_threads > 1) {
        int provided;
        MPI_Init_thread(NULL, NULL, MPI_THREAD_SERIALIZED, &provided);
        if (provided != MPI_THREAD_SERIALIZED) {
            std::cerr << "MPI does not support MPI_THREAD_SERIALIZED.\n";
            return 2;
        }
    }
    else {
        MPI_Init(NULL, NULL);
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // master sends file and worker receives file and stores it
    auto start_send = sc.now(), end_send = sc.now();
    if (rank == 0) {
        std::ifstream fin(argv[1], std::ios::binary);
        if (!fin) {
            std::cerr << "Cannot open file " << argv[1] << "\n";
            return 1;
        }
        fin.seekg(0, std::ios::end);
        size_t file_size = fin.tellg();
        fin.seekg(0, std::ios::beg);
        std::cout << "File size: " << file_size << "\n";
        char* buf = new char[file_size];
        fin.read(buf, file_size);
        fin.close();
        start_send = sc.now();
        for (int i = 1; i < size; i++) {
            MPI_Send(&file_size, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD);
            MPI_Send(buf, file_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
        end_send = sc.now();
        delete[] buf;
    } else {
        size_t file_size;
        MPI_Recv(&file_size, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        char* buf = new char[file_size];
        MPI_Recv(buf, file_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "rank: " << rank << " received file.\n";
        std::ofstream fout("tmp.bin", std::ios::binary);
        fout.write(buf, file_size);
        fout.close();
        delete[] buf;
        argv[1] = "tmp.bin";
    }
    auto time_span_send = static_cast<std::chrono::duration<double>>(end_send - start_send);

    Graph g = load_graph_binary(argv[1]);
    std::cout << "Load graph: " << argv[1] << "\n";
    const size_t n = g->num_nodes;
    std::cout << "Graph loaded.\n";
    std::cout << "num_nodes: " << n << "\n";

    auto ans = init_ans(n);

    int num_nodes_per_proc = n / size;

    MPI_Barrier(MPI_COMM_WORLD);

    auto start = sc.now(), end = sc.now();
    if (num_threads > 1) {
        std::cout << "Running mpi version with openmp.\n";
        start = sc.now();
        dijk_mpi_omp(g, ans, n, num_threads, rank, size);
        end = sc.now();
    }
    else {
        std::cout << "Running mpi version.\n";
        start = sc.now();
        dijk_mpi(g, ans, n, rank, size);
        end = sc.now();
    }
    auto time_span = static_cast<std::chrono::duration<double>>(end - start);

    std::cout << "answer done.\n";

    if (rank == 0) {
        auto ref_ans = init_ans(n);
        std::cout << "Calculating ref answer...\n";
        auto ref_start = sc.now(); 
        dijk_serial(g, ref_ans, n);
        auto ref_end = sc.now();
        auto ref_time_span = static_cast<std::chrono::duration<double>>(ref_end - ref_start);
        std::cout << "ref answer done.\n";

        // Verify ans
        bool correct = true;
        for (int i = 0; i < n; i++) {
            if (memcmp(ref_ans[i], ans[i], n * sizeof(ans[0][0])) != 0) {
                std::cerr << "\033[1;31mVerification failed.\033[0m\n";
                std::cerr << "Failed at " << i << ": " << ref_ans[i][i] << " " << ans[i][i] << "\n";
                correct = false;
                break;
            }
        }
        if (correct) std::cout << "\033[1;32mVerification passed.\033[0m\n";
        free_ans(ref_ans, n);
        output_summary(ref_time_span, time_span + time_span_send);
        std::cout << std::endl;
    }

    free_ans(ans, n);
    free_graph(g);

    return 0;
}
