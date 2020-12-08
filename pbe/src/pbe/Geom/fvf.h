#pragma once

#include <d3d12.h>
#include <stdint.h>
#include <vector>

namespace pbe {

	using FVF = uint64_t;

	const FVF FVF_UNKNOWN = -1;
	const FVF FVF_POS = 1 << 0;
	const FVF FVF_NORMAL = FVF_POS << 1;
	const FVF FVF_UV = FVF_NORMAL << 1;
	const FVF FVF_COLOR = FVF_UV << 1;
	const FVF FVF_LAST = FVF_COLOR << 1;


	uint fvfGetStride(FVF fvf);
	uint fvfGetOffset(FVF fvf, FVF type);
	std::vector<D3D12_INPUT_ELEMENT_DESC> fvfGetInputLayout(FVF fvf);

}
