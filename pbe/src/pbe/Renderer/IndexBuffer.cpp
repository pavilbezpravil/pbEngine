#include "pch.h"
#include "IndexBuffer.h"



namespace pbe {

	Ref<IndexBuffer> IndexBuffer::CreateIB(uint32_t size)
	{
		return IndexBuffer::CreateIB(nullptr, size);
	}

	Ref<IndexBuffer> IndexBuffer::CreateIB(const void* data, uint32_t size)
	{
		auto ib = Ref<IndexBuffer>::Create();
		auto indexByteSize = sizeof(uint);
		HZ_CORE_ASSERT(size % indexByteSize == 0);
		ib->Create(L"", size / indexByteSize, indexByteSize, data);
		return ib;
	}

}
