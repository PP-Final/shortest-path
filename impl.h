#include "common/graph.h"

void dijk_serial(Graph graph, int** ans, const int n);
void dijk_thread(Graph graph, int** ans, const int n);