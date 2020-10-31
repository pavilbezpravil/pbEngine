#pragma once

#include <glm/glm.hpp>

#include "Common.h"

namespace pbe {

	struct AABB
	{
		glm::vec3 Min, Max;

		AABB()
			: Min(FLT_MAX), Max(FLT_MIN) {}

		AABB(const glm::vec3& min, const glm::vec3& max)
			: Min(min), Max(max) {}

		void AddPoint(const Vec3& p) {
			Min = glm::min(Min, p);
			Max = glm::max(Max, p);
		}
	};


}
