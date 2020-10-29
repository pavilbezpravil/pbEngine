#pragma once

#include "pch.h"

// A faster version of memcopy that uses SSE instructions.
void SIMDMemCopy(void* __restrict _Dest, const void* __restrict _Source, size_t NumQuadwords);

void SIMDMemFill(void* __restrict _Dest, __m128 FillVector, size_t NumQuadwords);

std::wstring MakeWStr(const std::string& str);

std::string MakeStr(const std::wstring& str);