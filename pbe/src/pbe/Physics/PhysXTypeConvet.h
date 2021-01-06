#pragma once

#include "pbe/Core/Math/Common.h"

namespace pbe
{
	using namespace physx;

	Vec2 PxVec2ToPBE(const PxVec2& v);
	Vec3 PxVec3ToPBE(const PxVec3& v);
	Vec4 PxVec4ToPBE(const PxVec4& v);
	Quat PxQuatToPBE(const PxQuat& q);

	PxVec2 Vec2ToPx(const Vec2& v);
	PxVec3 Vec3ToPx(const Vec3& v);
	PxVec4 Vec4ToPx(const Vec4& v);
	PxQuat QuatToPx(const Quat& q);

}

