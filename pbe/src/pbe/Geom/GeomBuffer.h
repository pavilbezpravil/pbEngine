#pragma once

#include <vector>
#include "fvf.h"
#include "pbe/Core/Math/Common.h"


namespace pbe {


	struct GeomAccessor;

	struct GeomFace {
		uint16_t i0, i1, i2;
	};


	struct GeomBuffer {
		FVF fvf = FVF_UNKNOWN;
		int size = 0;
		std::vector<BYTE> data;
		int stride = 0;
		std::vector<GeomFace> faces;

		void Create(FVF fvf, int size, int faces);

		GeomAccessor& GetAccessor();
	};


	struct GeomAccessor {
		GeomBuffer& buffer;

		explicit GeomAccessor(GeomBuffer& buffer)
			: buffer(buffer) {
		}

		const BYTE* GetRaw(int i, FVF type, int typeIdx = 0) const;
		BYTE* GetRaw(int i, FVF type, int typeIdx = 0);

		const Vec3& GetPos(int i) const;
		Vec3& PosMut(int i);

		const Vec3& GetNormal(int i) const;
		Vec3& NormalMut(int i);

		int NumFace() const;
		const GeomFace& GetFace(int face) const;
		GeomFace& FaceMut(int face);
	};

}
