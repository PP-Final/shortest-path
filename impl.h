#ifndef __IMPL_H__
#define __IMPL_H__

#include "common/graph.h"

using Answer = int**;

void dijk_serial(Graph graph, Answer ans, const int n);
void dijk_thread(Graph graph, Answer ans, const int n);

#endif