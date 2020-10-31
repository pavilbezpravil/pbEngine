#include "pch.h"
#include "GeomUtils.h"

namespace pbe {

	Ref<VertexBuffer> GeomUtils::GeomCreateVertexBuffer(GeomBuffer& geomBuffer) {
		return VertexBuffer::CreateVB(geomBuffer.GetRawVertexData(),geomBuffer.NumVertex() * geomBuffer.GetStride(), geomBuffer.GetFVF());
	}

	Ref<IndexBuffer> GeomUtils::GeomCreateIndexBuffer(GeomBuffer& geomBuffer) {
		return IndexBuffer::CreateIB(geomBuffer.GetRawFaceData(), geomBuffer.NumFace() * sizeof(GeomFace));
	}

}
