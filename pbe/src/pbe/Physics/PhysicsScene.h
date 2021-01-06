#pragma once

#include "pbe/Core/Ref.h"
#include "pbe/Core/UUID.h"
#include "pbe/Core/Math/Common.h"

#include "Queries.h"

namespace pbe
{
	class Entity;
	class Scene;
	
	using namespace physx;
	
	namespace physics
	{

		void Init();
		void Term();

		
		class PhysicsScene : public RefCounted
		{
		public:
			PhysicsScene(PxScene* pScene);
			~PhysicsScene();

			void Simulation(float dt);

			void OnGeomConstruct(Entity entity, PxGeometryType::Enum geomType);
			void OnGeomDestroy(Entity entity, PxGeometryType::Enum geomType);

			void OnRigidbodyConstruct(Entity entity);
			void OnRigidbodyDestroy(Entity entity);

			void OnEntityTransformChanged(Entity entity);

			void SyncPhysicsWithScene();
			
			void RenderPhysicsInfo();

			bool GetSimulatePhysics() const { return simulatePhysics; }
			void SetSimulatePhysics(bool simulate);

			// Queries
			bool RayCast(Vec3 origin, Vec3 dir, float maxDistance,
				RaycastHit& hit, PxHitFlags hitFlags = PxHitFlags(PxHitFlag::eDEFAULT),
				const PxQueryFilterData& filterData = PxQueryFilterData());

			bool OverlapSphere(Vec3 center, float radius,
			                   OverlapHit& hit, const PxQueryFilterData& filterData = PxQueryFilterData());
			std::vector<OverlapHit> OverlapSphereAll(Vec3 center, float radius, 
				const PxQueryFilterData& filterData = PxQueryFilterData());

			// PxScene* GetPxScene() { return pScene; }
			
		private:
			friend Scene;
			
			PxScene* pScene;

			std::unordered_map<pbe::UUID, PxRigidActor*> actorsMap;
			std::unordered_map<pbe::UUID, uint> preActorShapesCount;

			bool ignoreTransformChanged = false;
			bool simulatePhysics = true;

			void DestroyAllEntities();

			PxRigidActor* CreateActor(Entity entity);
			void DestroyActor(Entity entity);

			void CreateGeoms(Entity entity);
			void DestroyGeoms(Entity entity);
			void RecreateGeom(Entity entity);
		};


		class Physics
		{
		public:
			Physics();
			~Physics();

			Ref<PhysicsScene> CreateScene();
			
			const PxMaterial& GetDefaultMaterial() const;

			// void sdf()
			// {
			// 	PxRigidDynamic* meshActor = getPhysics().createRigidDynamic(PxTransform(1.0f));
			// 	PxShape* meshShape;
			// 	if (meshActor)
			// 	{
			// 		meshActor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
			//
			// 		PxTriangleMeshGeometry triGeom;
			// 		triGeom.triangleMesh = triangleMesh;
			// 		meshShape = PxRigidActorExt::createExclusiveShape(*meshActor, triGeom,
			// 			defaultMaterial);
			// 		getScene().addActor(*meshActor);
			// 	}
			// }
			
			PxDefaultErrorCallback gDefaultErrorCallback;
			PxDefaultAllocator gDefaultAllocatorCallback;
			PxFoundation* foundation = NULL;
			PxPvd* mPvd = NULL;
			PxPhysics* mPhysics = NULL;
			PxDefaultCpuDispatcher* gDispatcher = NULL;

			PxMaterial* material = NULL;
		};
		extern Physics* s_Physics;

	}

}

