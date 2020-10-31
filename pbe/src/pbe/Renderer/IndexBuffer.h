#pragma once

#include "pbe/Renderer/GpuBuffer.h"

namespace pbe {

	class IndexBuffer : public ByteAddressBuffer
	{
	public:
		static Ref<IndexBuffer> CreateIB(uint32_t size);
		static Ref<IndexBuffer> CreateIB(const void* data, uint32_t size = 0);
	};

}

