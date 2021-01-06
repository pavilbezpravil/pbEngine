#include "pch.h"
#include "Utils.h"
#include "Queries.h"
#include "pbe/Scene/Scene.h"
#include "pbe/Scene/Entity.h"
#include "pbe/Renderer/RendPrim.h"

namespace pbe
{
	namespace physics
	{
		using namespace physx;

		bool SceneRayCast(Scene* pScene, Vec3 origin, Vec3 dir, float maxDistance, RaycastHit& hit,
			physx::PxHitFlags hitFlags, const physx::PxQueryFilterData& filterData)
		{
			bool status = pScene->GetPhysicsScene()->RayCast(origin, dir, maxDistance, hit, hitFlags, filterData);

			auto end = origin + dir * maxDistance;
			if (status) {
				RendPrim::DrawLine(origin, hit.position, Color_Green);
				RendPrim::DrawLine(hit.position, end, Color_Red);
			} else {
				RendPrim::DrawLine(origin, end, Color_Green);
			}
			
			return status;
		}

		std::vector<OverlapHit> SceneOverlapSphereAll(Scene* pScene, Vec3 center, float radius,
			const physx::PxQueryFilterData& filterData)
		{
			auto overlaped = pScene->GetPhysicsScene()->OverlapSphereAll(center, radius, filterData);
			RendPrim::DrawSphere(center, radius, 16, overlaped.empty() ? Color_Green : Color_Red);

			for (auto& hit : overlaped) {
				auto& trans = hit.entity->GetComponent<TransformComponent>();
				RendPrim::DrawLine(center, trans.WorldPosition(), Color_Red);
			}

			return overlaped;
		}
	}
}
