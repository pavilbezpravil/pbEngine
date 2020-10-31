#include "pch.h"
#include "VertexBuffer.h"

#include "Renderer.h"

namespace pbe {

	Ref<VertexBuffer> VertexBuffer::CreateVB(const void* data, uint32_t size, FVF fvf)
	{
		auto vb = Ref<VertexBuffer>::Create();
		vb->_fvf = fvf;
		auto stride = fvfGetStride(fvf);
		vb->Create(L"", size / stride, stride, data);
		return vb;
	}

	Ref<VertexBuffer> VertexBuffer::CreateVB(uint32_t size, FVF fvf)
	{
		return CreateVB(nullptr, size, fvf);
	}
}
