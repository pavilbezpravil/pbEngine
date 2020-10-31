#pragma once

#include <vector>
#include "fvf.h"
#include "pbe/Core/Math/Common.h"


namespace pbe {

	struct GeomFace {
		uint i0, i1, i2;
	};

	static_assert(sizeof(GeomFace) == 3 * sizeof(uint));

	struct GeomBuffer {
		void Create(FVF fvf, int size = 0, int faces = 0);

		void AddVertex();
		void AddFace();

		FVF GetFVF() const { return fvf; }
		int NumVertex() const { return size; }
		int GetStride() const { return stride; }
		const void* GetRawVertexData() const { return data.data(); }
		const void* GetRawFaceData() const { return faces.data(); }

		const BYTE* GetRaw(int i, FVF type, int typeIdx = 0) const;
		BYTE* GetRaw(int i, FVF type, int typeIdx = 0);

		const Vec3& GetPos(int i) const;
		Vec3& PosMut(int i);

		const Vec3& GetNormal(int i) const;
		Vec3& NormalMut(int i);

		int NumFace() const;

		const GeomFace& GetFace(int face) const;
		GeomFace& FaceMut(int face);

	private:
		FVF fvf = FVF_UNKNOWN;
		int size = 0;
		std::vector<byte> data;
		int stride = 0;
		std::vector<GeomFace> faces;
	};

}
