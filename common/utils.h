#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdint>

using Distance = int;
using Answer = Distance**;

void output_result(const Answer ans, const int n);

template <class T1, class T2>
void output_summary(T1 ref, T2 impl);

Answer init_ans(size_t n);

void free_ans(Answer ans);

#endif