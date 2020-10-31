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

	private:
		FVF _fvf;
	};

}
