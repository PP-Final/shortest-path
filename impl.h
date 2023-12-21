#include <vector>
#include "common/graph.h"

void dijk_serial(Graph graph, std::vector<std::vector<int>> &ans, const int n);
void dijk_thread(Graph graph, std::vector<std::vector<int>> &ans, const int n);