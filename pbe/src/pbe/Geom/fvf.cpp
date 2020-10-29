#include "pch.h"
#include "fvf.h"

namespace pbe {

	namespace
	{
		int fvf_size_map[]
		{
			 3 * 4, 3 * 4, 2 * 4
		};
	}

	uint fvfGetStride(FVF fvf) {
		int stride = 0;
		if (fvf & FVF_POS) {
			stride += fvf_size_map[0];
		}
		if (fvf & FVF_NORMAL) {
			stride += fvf_size_map[1];
		}
		if (fvf & FVF_UV) {
			stride += fvf_size_map[2];
		}
		return stride;
	}

	uint fvfGetOffset(FVF fvf, FVF type) {
		ASSERT(fvf & type);
		int offset = 0;
		for (int i = 0; ; ++i) {
			FVF checkedFVF = FVF_POS << i;
			if (checkedFVF >= type || checkedFVF >= FVF_LAST) {
				break;
			}
			if (fvf & checkedFVF) {
				offset += fvf_size_map[i];
			}
		}

		return offset;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> fvfGetInputLayout(FVF fvf) {
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

		if (fvf & FVF_POS) {
			inputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT     , 0, fvfGetOffset(fvf, FVF_POS), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		}
		if (fvf & FVF_NORMAL) {
			inputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT     , 0, fvfGetOffset(fvf, FVF_NORMAL), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		}

		return inputLayout;
	}

}