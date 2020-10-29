#pragma once
#include "GeomBuffer.h"
#include "pbe/Renderer/GpuBuffer.h"


namespace pbe {

	namespace GeomUtils
	{
		ByteAddressBuffer GeomCreateVertexBuffer(GeomBuffer& geomBuffer);
		ByteAddressBuffer GeomCreateIndexBuffer(GeomBuffer& geomBuffer);
	};

}
