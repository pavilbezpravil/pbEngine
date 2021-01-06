#pragma once
#include "pbe/Core/Math/Common.h"


namespace pbe
{
	class Entity;
	class Scene;

	namespace physics
	{
		struct RaycastHit;
		struct OverlapHit;

		bool SceneRayCast(Scene* pScene, Vec3 origin, Vec3 dir, float maxDistance,
		                  RaycastHit& hit, physx::PxHitFlags hitFlags = physx::PxHitFlags(physx::PxHitFlag::eDEFAULT),
		                  const physx::PxQueryFilterData& filterData = physx::PxQueryFilterData());
		
		std::vector<OverlapHit> SceneOverlapSphereAll(Scene* pScene, Vec3 center, float radius,
			const physx::PxQueryFilterData& filterData = physx::PxQueryFilterData());
		
	}
}

