// Dijkstra_kernel.cl

#define BITS_PER_UINT 64
#define UINT16_MAX 65535

typedef int Vertex;
typedef int Weight;
typedef int Distance;

typedef struct {
    const int num_edges;
    const int num_nodes;
    __global int* outgoing_starts;
    __global Vertex* outgoing_edges;
    
    __global int* incoming_starts;
    __global Vertex* incoming_edges;

    __global Weight* edges_weight;
} Graph;

__global const Vertex* outgoing_begin(const Graph* g, Vertex v) {
    return &g->outgoing_edges[g->outgoing_starts[v]];
}

// Function to get the pointer to the beginning of edge weights for a vertex
__global const Vertex* weight_begin(const Graph* g, Vertex v) {
    return &g->edges_weight[g->outgoing_starts[v]];
}

// Function to get the size of outgoing edges for a vertex
int outgoing_size(const Graph* g, Vertex v) {
    if (v == g->num_nodes - 1) {
        return g->num_edges - g->outgoing_starts[v];
    } else {
        return g->outgoing_starts[v + 1] - g->outgoing_starts[v];
    }
}

void setVisited(unsigned long* visited, int node) {
    visited[node / BITS_PER_UINT] |= (1UL << (node % BITS_PER_UINT));
    barrier(CLK_LOCAL_MEM_FENCE);
}

bool isVisited(unsigned long* visited, int node) {
    return (visited[node / BITS_PER_UINT] & (1UL << (node % BITS_PER_UINT))) != 0;
}

__kernel void dijkstra_kernel(
    const int num_edges, 
    const int num_nodes,
    __global int* outgoing_starts,
    __global Vertex* outgoing_edges,
    __global int* incoming_starts,
    __global Vertex* incoming_edges,
    __global Weight* edges_weight,

    __global Distance* ans, const int n,
    __local unsigned long* visited
) {
    Graph graph = {
        .num_edges = num_edges,
        .num_nodes = num_nodes,
        .outgoing_starts = outgoing_starts,
        .outgoing_edges = outgoing_edges,
        .incoming_starts = incoming_starts,
        .incoming_edges = incoming_edges,
        .edges_weight = edges_weight
    };

    for(int i = 0; i <= n / BITS_PER_UINT + 1; i++) {
        visited[i] = 0UL;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    
    int id = get_global_id(0);
    for(int i = id; i < n; i += n) {
        ans[i * n + i] = 0;
        for (int j = 0; j < n; j++) {
            int u = -1;
            for (int k = 0; k < n; k++) {
                if (!isVisited(visited, k) && (u == -1 || ans[i * n + k] < ans[i * n + u])) {
                    u = k;
                }
            }

            setVisited(visited, u);

            // Relax neighboring nodes
            for (int k = 0; k < outgoing_size(&graph, u); k++) {
                int v = outgoing_begin(&graph, u)[k];
                if (ans[i * n + u] + 1 < ans[i * n + v]) {
                    ans[i * n + v] = ans[i * n + u] + 1;
                }
            }
        }
    }
}