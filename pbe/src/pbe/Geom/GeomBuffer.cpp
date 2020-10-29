#include "pch.h"
#include "GeomBuffer.h"

namespace pbe {

	void GeomBuffer::Create(FVF fvf, int size, int faces) {
		this->fvf = fvf;
		this->size = size;
		stride = fvfGetStride(fvf);
		data.resize(stride * size);

		this->faces.resize(faces);
	}

	GeomAccessor& GeomBuffer::GetAccessor() {
		return GeomAccessor{ *this };
	}

	const BYTE* GeomAccessor::GetRaw(int i, FVF type, int typeIdx) const {
		return &buffer.data[i * buffer.stride + fvfGetOffset(buffer.fvf, type << typeIdx)];
	}

	BYTE* GeomAccessor::GetRaw(int i, FVF type, int typeIdx) {
		return &buffer.data[i * buffer.stride + fvfGetOffset(buffer.fvf, type << typeIdx)];
	}

	const Vec3& GeomAccessor::GetPos(int i) const {
		return *(const Vec3*)(GetRaw(i, FVF_POS));
	}

	Vec3& GeomAccessor::PosMut(int i) {
		return *(Vec3*)(GetRaw(i, FVF_POS));
	}

	const Vec3& GeomAccessor::GetNormal(int i) const {
		return *(const Vec3*)(GetRaw(i, FVF_NORMAL));
	}

	Vec3& GeomAccessor::NormalMut(int i) {
		return *(Vec3*)(GetRaw(i, FVF_NORMAL));
	}

	int GeomAccessor::NumFace() const {
		return buffer.faces.size();
	}

	const GeomFace& GeomAccessor::GetFace(int face) const {
		return buffer.faces[face];
	}

	GeomFace& GeomAccessor::FaceMut(int face) {
		return buffer.faces[face];
	}

}