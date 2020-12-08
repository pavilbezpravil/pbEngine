#pragma once

#include <glm/glm.hpp>

#include "Common.h"

namespace pbe {

	struct AABB
	{
		Vec3 Min, Max;

		AABB()
			: Min(FLT_MAX), Max(FLT_MIN) {}

		AABB(const Vec3& min, const Vec3& max)
			: Min(min), Max(max) {}

		static AABB FromCenterHalfSize(const Vec3& center, const Vec3& halfSize)
		{
			return AABB{center - halfSize, center + halfSize};
		}

		Vec3 Size() const { return Max - Min; }

		void AddPoint(const Vec3& p) {
			Min = glm::min(Min, p);
			Max = glm::max(Max, p);
		}
	};


}
