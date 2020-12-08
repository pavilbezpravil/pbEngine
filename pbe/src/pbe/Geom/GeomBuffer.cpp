#include "pch.h"
#include "GeomBuffer.h"

namespace pbe {

	void GeomBuffer::Create(FVF fvf, int size, int nIndexes) {
		this->fvf = fvf;
		stride = fvfGetStride(fvf);
		data.resize(stride * size);

		this->indexes.resize(nIndexes);
	}

	void GeomBuffer::Clear()
	{
		data.clear();
		indexes.clear();
	}

	void GeomBuffer::AddVertex()
	{
		AddVertexes(1);
	}

	void GeomBuffer::AddVertexes(uint n)
	{
		HZ_CORE_ASSERT(stride > 0);
		data.resize(data.size() + stride * n);
	}

	void GeomBuffer::AddIndexes(uint n)
	{
		HZ_CORE_ASSERT(stride > 0);
		indexes.resize(indexes.size() + n);
	}

	void GeomBuffer::AddFace()
	{
		HZ_CORE_ASSERT(stride > 0);
		AddIndexes(3);
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

	const Vec4& GeomBuffer::GetColor(int i) const
	{
		return *(const Vec4*)(GetRaw(i, FVF_COLOR));
	}

	Vec4& GeomBuffer::ColorMut(int i)
	{
		return *(Vec4*)(GetRaw(i, FVF_COLOR));
	}

	uint GeomBuffer::NumIndexes() const
	{
		return uint(indexes.size());
	}

	const uint& GeomBuffer::GetIndex(int i) const
	{
		return indexes[i];
	}

	uint& GeomBuffer::IndexMut(int i)
	{
		return indexes[i];
	}

	uint GeomBuffer::NumFace() const {
		// todo: check
		HZ_CORE_ASSERT(NumIndexes() % 3 == 0);
		return NumIndexes() / 3;
	}

	const GeomFace& GeomBuffer::GetFace(int face) const {
		return *(GeomFace*)&indexes[face * 3];
	}

	GeomFace& GeomBuffer::FaceMut(int face) {
		return *(GeomFace*)&indexes[face * 3];
	}

}
