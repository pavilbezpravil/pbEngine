#pragma once
#include "pbe/Core/Singleton.h"

namespace pbe {

	class Allocator
	{
	public:
		void* alloc(size_t size) {
			_totalAllocBytes += size;
			_curAllocBytes += size;
			++_totalNAlloc;
			++_curNAlloc;
			void* p = malloc(size + sizeof(uint));
			*(uint*)p = size;
			return (byte*)p + sizeof(uint);
			// return malloc(size);
		}

		void free(void* p) {
			p = (byte*)p - sizeof(uint);
			uint size = *(uint*)p;
			_curAllocBytes -= size;
			--_curNAlloc;
			::free(p);
		}

		uint64 TotalAllocBytes() const { return _totalAllocBytes; }
		uint64 CurAllocBytes() const { return _curAllocBytes; }

		uint64 TotalAllocCount() const { return _totalNAlloc; }
		uint64 CurAllocCount() const { return _curNAlloc; }
		
	private:
		uint64 _totalNAlloc = 0;
		uint64 _totalAllocBytes = 0;
		uint64 _curNAlloc = 0;
		uint64 _curAllocBytes = 0;
	};

	Allocator pbeAllocator;

}

inline void* operator new(size_t size)
{
	return pbe::pbeAllocator.alloc(size);
}

inline void operator delete(void* p)
{
	if (p)
		pbe::pbeAllocator.free(p);
}
