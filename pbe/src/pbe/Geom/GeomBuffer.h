#pragma once

#include <vector>
#include "fvf.h"
#include "pbe/Core/Math/Common.h"


namespace pbe {

	struct GeomFace {
		uint i0, i1, i2;
	};

	static_assert(sizeof(GeomFace) == 3 * sizeof(uint));

	using IndexType = uint;
	#define SIZEOF_INDEX sizeof(IndexType)
	
	struct GeomBuffer {
		void Create(FVF fvf, int size = 0, int nIndexes = 0);
		void Clear();

		void AddVertex();
		void AddVertexes(uint n);
		void AddIndexes(uint n);
		void AddFace();

		FVF GetFVF() const { return fvf; }
		uint NumVertex() const { return uint(data.size()) / stride; }
		uint GetStride() const { return stride; }
		const void* GetRawVertexData() const { return data.data(); }
		const void* GetRawIndexesData() const { return indexes.data(); }

		const BYTE* GetRaw(int i, FVF type, int typeIdx = 0) const;
		BYTE* GetRaw(int i, FVF type, int typeIdx = 0);

		const Vec3& GetPos(int i) const;
		Vec3& PosMut(int i);

		const Vec3& GetNormal(int i) const;
		Vec3& NormalMut(int i);

		const Vec4& GetColor(int i) const;
		Vec4& ColorMut(int i);

		uint NumIndexes() const;
		const uint& GetIndex(int i) const;
		uint& IndexMut(int i);

		uint NumFace() const;
		const GeomFace& GetFace(int face) const;
		GeomFace& FaceMut(int face);

	private:
		FVF fvf = FVF_UNKNOWN;
		// int size = 0;
		std::vector<byte> data;
		uint stride = 0;
		std::vector<uint> indexes;
	};

}
