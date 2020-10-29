#pragma once
#include "GeomBuffer.h"

namespace pbe {

	namespace GeomPrimitive
	{
		GeomBuffer CreateBox(FVF fvf, float width, float height, float depth, uint numSubdivisions);
	};

}
