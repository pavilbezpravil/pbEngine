#include "pch.h"
#include "GeomUtils.h"

namespace pbe {

	ByteAddressBuffer GeomUtils::GeomCreateVertexBuffer(GeomBuffer& geomBuffer) {
		ByteAddressBuffer ret;
		ret.Create(L"GeomCreateVertexBuffer", geomBuffer.size, geomBuffer.stride, (void*)geomBuffer.data.data());
		return ret;
	}

	ByteAddressBuffer GeomUtils::GeomCreateIndexBuffer(GeomBuffer& geomBuffer) {
		ByteAddressBuffer ret;
		ret.Create(L"GeomCreateIndexBuffer", geomBuffer.faces.size(), sizeof(GeomFace), (void*)geomBuffer.faces.data());
		return ret;
	}

}