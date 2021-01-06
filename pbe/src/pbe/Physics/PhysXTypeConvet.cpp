#include "pch.h"
#include "PhysXTypeConvet.h"

namespace pbe
{
	Vec2 PxVec2ToPBE(const PxVec2& v)
	{
		return Vec2{v.x, v.y};
	}

	Vec3 PxVec3ToPBE(const PxVec3& v)
	{
		return Vec3{ v.x, v.y, v.z };
	}

	Vec4 PxVec4ToPBE(const PxVec4& v)
	{
		return Vec4{ v.x, v.y, v.z, v.w };
	}

	Quat PxQuatToPBE(const PxQuat& q)
	{
		return Quat{ q.w, q.x, q.y, q.z };
	}

	PxVec2 Vec2ToPx(const Vec2& v)
	{
		return PxVec2{ v.x, v.y };
	}

	PxVec3 Vec3ToPx(const Vec3& v)
	{
		return PxVec3{ v.x, v.y, v.z };
	}

	PxVec4 Vec4ToPx(const Vec4& v)
	{
		return PxVec4{ v.x, v.y, v.z, v.w };
	}

	PxQuat QuatToPx(const Quat& q)
	{
		return PxQuat{ q.x, q.y, q.z, q.w };
	}
}
