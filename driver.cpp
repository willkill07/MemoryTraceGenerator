#include <iostream>

#include "memory_trace.hpp"
#include "multi_array.hpp"

mem::AccessType mem::trace_storage::list = mem::AccessType {};

int main () {

	using Type = int;
	const int M = 2;
	const int N = 4;
	const int P = 2;

	auto A = mem::make_trace (multi_array <Type, M, N>());
	auto B = mem::make_trace (multi_array <Type, N, P>());
	auto C = mem::make_trace (multi_array <Type, M, P>());

	for (auto i = size_t{0}; i < M; ++i)
		for (auto j = size_t{0}; j < N; ++j)
			A[i][j] = rand() % 8;

 	for (auto j = size_t{0}; j < N; ++j)
		for (auto k = size_t{0}; k < P; ++k)
			B[j][k] = rand() % 8;

	for (auto i = size_t{0}; i < M; ++i)
		for (auto k = size_t{0}; k < P; ++k)
			C[i][k] = 0;

	for (auto i = size_t{0}; i < M; ++i)
		for (auto j = size_t{0}; j < N; ++j)
			for (auto k = size_t{0}; k < P; ++k)
				C[i][k] += A[i][j] * B[j][k];

	for (auto i = size_t{0}; i < M; ++i)
		for (auto k = size_t{0}; k < P; ++k)
			C[i][k] = C[i][k];

	for (const auto & i : mem::trace_storage::list)
		std::cout << i << std::endl;

	return EXIT_SUCCESS;
}
