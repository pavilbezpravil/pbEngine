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

	void GeomBuffer::AddVertex()
	{
		data.resize(data.size() + stride);
	}

	void GeomBuffer::AddFace()
	{
		data.resize(data.size() + 1);
	}

	const BYTE* GeomBuffer::GetRaw(int i, FVF type, int typeIdx) const {
		return &data[i * stride + fvfGetOffset(fvf, type << typeIdx)];
	}

	BYTE* GeomBuffer::GetRaw(int i, FVF type, int typeIdx) {
		return &data[i * stride + fvfGetOffset(fvf, type << typeIdx)];
	}

	const Vec3& GeomBuffer::GetPos(int i) const {
		return *(const Vec3*)(GetRaw(i, FVF_POS));
	}

	Vec3& GeomBuffer::PosMut(int i) {
		return *(Vec3*)(GetRaw(i, FVF_POS));
	}

	const Vec3& GeomBuffer::GetNormal(int i) const {
		return *(const Vec3*)(GetRaw(i, FVF_NORMAL));
	}

	Vec3& GeomBuffer::NormalMut(int i) {
		return *(Vec3*)(GetRaw(i, FVF_NORMAL));
	}

	int GeomBuffer::NumFace() const {
		return faces.size();
	}

	const GeomFace& GeomBuffer::GetFace(int face) const {
		return faces[face];
	}

	GeomFace& GeomBuffer::FaceMut(int face) {
		return faces[face];
	}

}
