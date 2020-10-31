#include "pch.h"
#include "IndexBuffer.h"

#include "../../../Types.h"

namespace pbe {

	Ref<IndexBuffer> IndexBuffer::CreateIB(uint32_t size)
	{
		return IndexBuffer::CreateIB(nullptr, size);
	}

	Ref<IndexBuffer> IndexBuffer::CreateIB(const void* data, uint32_t size)
	{
		auto ib = Ref<IndexBuffer>::Create();
		auto faceByteSize = sizeof(uint);
		HZ_CORE_ASSERT(size % faceByteSize == 0);
		ib->Create(L"", size / faceByteSize, faceByteSize, data);
		return ib;
	}

}
