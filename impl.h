#ifndef __IMPL_H__
#define __IMPL_H__

#include "common/graph.h"
#include "common/utils.h"

void dijk_serial(Graph graph, Answer ans, const int n);
void dijk_thread(Graph graph, Answer ans, const int n, const int num_threads);
void dijk_mp(Graph graph, Answer ans, const int n, const int num_threads);
// void dijk_mpi(Graph graph, Answer ans, const int n);
// void dijk_mpi_omp(Graph graph, Answer ans, const int n, const int num_threads);

#endif