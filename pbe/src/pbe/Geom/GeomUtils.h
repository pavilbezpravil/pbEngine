#pragma once
#include "GeomBuffer.h"
#include "pbe/Renderer/GpuBuffer.h"
#include "pbe/Renderer/IndexBuffer.h"
#include "pbe/Renderer/VertexBuffer.h"


namespace pbe {

	namespace GeomUtils
	{
		Ref<VertexBuffer> GeomCreateVertexBuffer(GeomBuffer& geomBuffer);
		Ref<IndexBuffer> GeomCreateIndexBuffer(GeomBuffer& geomBuffer);
	};

}
