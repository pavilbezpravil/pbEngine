#pragma once

#include "pbe/Geom/fvf.h"
#include "pbe/Renderer/GpuBuffer.h"

namespace pbe {

	class VertexBuffer : public ByteAddressBuffer
	{
	public:
		static Ref<VertexBuffer> CreateVB(const void* data, uint32_t size, FVF fvf);
		static Ref<VertexBuffer> CreateVB(uint32_t size, FVF fvf);

		FVF GetFVF() const { return _fvf; }
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputLayout() const { return _inputLayout; }

	private:
		FVF _fvf;
		std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayout;
	};

}
