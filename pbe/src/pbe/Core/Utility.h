#pragma once

#include "pch.h"

// A faster version of memcopy that uses SSE instructions.
void SIMDMemCopy(void* __restrict _Dest, const void* __restrict _Source, size_t NumQuadwords);

void SIMDMemFill(void* __restrict _Dest, __m128 FillVector, size_t NumQuadwords);

std::wstring MakeWStr(const std::string& str);

std::string MakeStr(const std::wstring& str);

template<typename T>
int vector_find(const std::vector<T>& vec, const T& element)
{
	for (int i = 0; i < vec.size(); ++i) {
		if (vec[i] == element) {
			return i;
		}
	}
	return -1;
}

template<typename T>
void vector_fast_erase(std::vector<T>& vec, int pos)
{
	vec[pos] = vec.back();
	vec.pop_back();
}
