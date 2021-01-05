#include "pch.h"
#include "PhysicsScene.h"
#include "pbe/Scene/Entity.h"

#include <PxPhysicsAPI.h>

#include "pbe/Renderer/RendPrim.h"

namespace pbe
{

	namespace physics
	{

		Physics* s_Physics = NULL;

		void Init()
		{
			s_Physics = new Physics;
		}

		void Term()
		{
			delete s_Physics;
		}

		PhysicsScene::PhysicsScene(PxScene* pScene) : pScene(pScene)
		{
			pScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			pScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
		}

		PhysicsScene::~PhysicsScene()
		{
			DestroyAllEntities();

			pScene->release();
		}

		void PhysicsScene::Simulation(float dt)
		{
			// mAccumulator = 0.0f;
			// mStepSize = 1.0f / 60.0f;
			//
			// mAccumulator += dt;
			// if (mAccumulator < mStepSize)
			// 	return false;
			//
			// mAccumulator -= mStepSize;
			//
			// mScene->simulate(mStepSize);
			if (simulatePhysics) {
				pScene->simulate(dt);
				pScene->fetchResults(true); // end simulation, execute all callbacks	
			}
		}

		PxRigidActor* PhysicsScene::CreateActor(Entity entity)
		{
			const auto& trans = entity.GetComponent<TransformComponent>();

			PxTransform pxTrans = PxTransform(Vec3ToPx(trans.WorldPosition()), QuatToPx(trans.WorldRotation()));
			
			// todo: consider scale
			bool hasRigidbody = entity.HasComponent<RigidbodyComponent>() && !entity.GetComponent<RigidbodyComponent>()._pandingForDestroy;
			
			PxRigidActor* actor;
			if (hasRigidbody) {
				actor = s_Physics->mPhysics->createRigidDynamic(pxTrans);
				entity.GetComponent<RigidbodyComponent>()._actor = actor;
			} else {
				actor = s_Physics->mPhysics->createRigidStatic(pxTrans);
			}
			
			// actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
			actor->userData = new Entity(entity);
			pScene->addActor(*actor);

			return actor;
		}
		
		void PhysicsScene::DestroyActor(Entity entity)
		{
			UUID uuid = entity.GetUUID();
			PxRigidActor* actor = actorsMap[uuid];

			Entity* userDataEntity = (Entity*)actor->userData;
			HZ_CORE_ASSERT(*userDataEntity == entity);
			delete userDataEntity;
			actor->release();
			actorsMap.erase(uuid);
			preActorShapesCount.erase(uuid);
		}

		void PhysicsScene::CreateGeoms(Entity entity)
		{
			#define ADD_COMPONENT_TO_ACTOR(ComponentType, GeomType) \
			if (entity.HasComponent<ComponentType>()) { \
				OnGeomConstruct(entity, PxGeometryType::GeomType); \
			}

			ADD_COMPONENT_TO_ACTOR(BoxColliderComponent, eBOX);
			ADD_COMPONENT_TO_ACTOR(SphereColliderComponent, eSPHERE);

			#undef ADD_COMPONENT_TO_ACTOR
		}

		void PhysicsScene::DestroyGeoms(Entity entity)
		{
			#define REMOVE_COMPONENT_FROM_ACTOR(ComponentType, GeomType) \
			if (entity.HasComponent<ComponentType>()) { \
				OnGeomDestroy(entity, PxGeometryType::GeomType); \
			}

			REMOVE_COMPONENT_FROM_ACTOR(BoxColliderComponent, eBOX);
			REMOVE_COMPONENT_FROM_ACTOR(SphereColliderComponent, eSPHERE);

			#undef REMOVE_COMPONENT_FROM_ACTOR

			HZ_CORE_ASSERT(actorsMap.find(entity.GetUUID()) == actorsMap.end());
			HZ_CORE_ASSERT(preActorShapesCount.find(entity.GetUUID()) == preActorShapesCount.end());
		}

		void PhysicsScene::RecreateGeom(Entity entity)
		{
			DestroyGeoms(entity);
			CreateGeoms(entity);
		}

		void PhysicsScene::OnGeomConstruct(Entity entity, PxGeometryType::Enum geomType)
		{
			if (preActorShapesCount.find(entity.GetUUID()) == preActorShapesCount.end()) {
				preActorShapesCount[entity.GetUUID()] = 0;
				actorsMap[entity.GetUUID()] = CreateActor(entity);
			}

			++preActorShapesCount[entity.GetUUID()];

			#define ADD_CASE(GeomType, ComponentType, GeomCreateCode) \
			case PxGeometryType::GeomType: \
			{ \
				auto& geom = entity.GetComponent<ComponentType>(); \
				/* HZ_CORE_ASSERT(!geom._shape); */ \
				PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, GeomCreateCode, s_Physics->GetDefaultMaterial()); \
				HZ_CORE_ASSERT(shape); \
				geom._shape = shape; \
				shape->setLocalPose(PxTransform(Vec3ToPx(geom.Center))); \
			} \
			break
			
			PxRigidActor* actor = actorsMap[entity.GetUUID()];
			switch (geomType) {
				ADD_CASE(eSPHERE, SphereColliderComponent, PxSphereGeometry(geom.Radius));
				// ADD_CASE(eSPHERE, PlaneColliderComponent, PxPlaneGeometry());
				// case PxGeometryType::ePLANE: break;
				// case PxGeometryType::eCAPSULE: break;
				ADD_CASE(eBOX, BoxColliderComponent, PxBoxGeometry(Vec3ToPx(geom.Size) * 0.5f));
				// case PxGeometryType::eCONVEXMESH: break;
				// case PxGeometryType::eTRIANGLEMESH: break;
				// case PxGeometryType::eHEIGHTFIELD: break;
				// case PxGeometryType::eGEOMETRY_COUNT: break;
				// case PxGeometryType::eINVALID: break;
				default: HZ_UNIMPLEMENTED();
			}
			#undef ADD_CASE
		}

		void PhysicsScene::OnGeomDestroy(Entity entity, PxGeometryType::Enum geomType)
		{
			HZ_CORE_ASSERT(preActorShapesCount.find(entity.GetUUID()) != preActorShapesCount.end());

			#define REMOVE_CASE(GeomType, ComponentType) \
			case PxGeometryType::GeomType: \
			{ \
				auto& geom = entity.GetComponent<ComponentType>(); \
				HZ_CORE_ASSERT(geom._shape); \
				actor->detachShape(*geom._shape); \
				geom._shape = NULL; \
			} \
			break
			
			PxRigidActor* actor = actorsMap[entity.GetUUID()];
			switch (geomType) {
				REMOVE_CASE(eSPHERE, SphereColliderComponent);
					// case PxGeometryType::ePLANE: break;
					// case PxGeometryType::eCAPSULE: break;
				REMOVE_CASE(eBOX, BoxColliderComponent);
				// case PxGeometryType::eCONVEXMESH: break;
				// case PxGeometryType::eTRIANGLEMESH: break;
				// case PxGeometryType::eHEIGHTFIELD: break;
				// case PxGeometryType::eGEOMETRY_COUNT: break;
				// case PxGeometryType::eINVALID: break;
				default: HZ_UNIMPLEMENTED();
			}
			#undef REMOVE_CASE
			
			uint count = --preActorShapesCount[entity.GetUUID()];
			
			if (count == 0) {
				DestroyActor(entity);
			}
		}

		void PhysicsScene::OnRigidbodyConstruct(Entity entity)
		{
			if (actorsMap.find(entity.GetUUID()) == actorsMap.end()) {
				HZ_CORE_ASSERT(preActorShapesCount.find(entity.GetUUID()) == preActorShapesCount.end());
				return;
			}
			HZ_CORE_ASSERT(preActorShapesCount.find(entity.GetUUID()) != preActorShapesCount.end());

			// HZ_CORE_ASSERT(!entity.GetComponent<RigidbodyComponent>()._actor);			
			RecreateGeom(entity);
		}

		void PhysicsScene::OnRigidbodyDestroy(Entity entity)
		{
			if (actorsMap.find(entity.GetUUID()) == actorsMap.end()) {
				HZ_CORE_ASSERT(preActorShapesCount.find(entity.GetUUID()) == preActorShapesCount.end());
				return;
			}
			HZ_CORE_ASSERT(preActorShapesCount.find(entity.GetUUID()) != preActorShapesCount.end());

			RecreateGeom(entity);
		}

		void PhysicsScene::OnEntityTransformChanged(Entity entity)
		{
			if (ignoreTransformChanged) {
				return;
			}

			auto iter = actorsMap.find(entity.GetUUID());
			if (iter != actorsMap.end()) {
				auto& trans = entity.GetComponent<TransformComponent>();
				PxTransform pxTrans{Vec3ToPx(trans.WorldPosition()), QuatToPx(trans.WorldRotation())};
				PxRigidActor* actor = iter->second;
				actor->setGlobalPose(pxTrans);
			}
		}

		void PhysicsScene::SyncPhysicsWithScene()
		{
			PxU32 nbActiveActors;
			PxActor** activeActors = pScene->getActiveActors(nbActiveActors);

			ignoreTransformChanged = true;
			for (PxU32 i = 0; i < nbActiveActors; ++i) {
				Entity entity = *static_cast<Entity*>(activeActors[i]->userData);
				auto& trans = entity.GetComponent<TransformComponent>();

				PxRigidActor* rbActor = activeActors[i]->is<PxRigidActor>();
				HZ_CORE_ASSERT(rbActor, "WTF is it mean?");
				if (rbActor) {
					PxTransform pxTrans = rbActor->getGlobalPose();
					trans.UpdatePosition(PxVec3ToPBE(pxTrans.p), Space::World);
					trans.UpdateRotation(PxQuatToPBE(pxTrans.q), Space::World);
				}
			}
			ignoreTransformChanged = false;
		}

		void PhysicsScene::RenderPhysicsInfo()
		{
			const PxRenderBuffer& rb = pScene->getRenderBuffer();
			for (PxU32 i = 0; i < rb.getNbPoints(); i++) {
				const PxDebugPoint& point = rb.getPoints()[i];
				Vec3 start = PxVec3ToPBE(point.pos);
				RendPrim::DrawLine(start, start + Vec3_Y, Color_Green);
			}
			for (PxU32 i = 0; i < rb.getNbLines(); i++) {
				const PxDebugLine& line = rb.getLines()[i];
				RendPrim::DrawLine(PxVec3ToPBE(line.pos0), PxVec3ToPBE(line.pos1), Color_Green);
			}
			for (PxU32 i = 0; i < rb.getNbTriangles(); i++) {
				const PxDebugTriangle& triangle = rb.getTriangles()[i];
				RendPrim::DrawLine(PxVec3ToPBE(triangle.pos0), PxVec3ToPBE(triangle.pos1), Color_Green);
			}
		}

		void PhysicsScene::SetSimulatePhysics(bool simulate)
		{
			simulatePhysics = simulate;
		}

		void PhysicsScene::DestroyAllEntities()
		{
			std::vector<Entity> entities;
			for (auto& [uuid, entity] : actorsMap) {
				entities.push_back(*(Entity*)entity->userData);
			}
			for (auto uuid : entities) {
				DestroyActor(uuid);
			}
		}

		Physics::Physics()
		{
			HZ_CORE_INFO("Physics: Init");
			
			foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback,
			                                gDefaultErrorCallback);
			if (!foundation)
				HZ_CORE_ASSERT(FALSE, "PxCreateFoundation failed!");

			HZ_CORE_INFO("Physics: PxFoundation created");
			
			bool recordMemoryAllocations = true;

			mPvd = PxCreatePvd(*foundation);
			char PVD_HOST[] = "localhost";
			PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
			mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

			HZ_CORE_INFO("Physics: physics visual debugge connected");

			mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation,
			                           PxTolerancesScale(), recordMemoryAllocations, mPvd);
			if (!mPhysics)
				HZ_CORE_ASSERT(FALSE, "PxCreatePhysics failed!");

			HZ_CORE_INFO("Physics: PxPhysics created");

			gDispatcher = PxDefaultCpuDispatcherCreate(4); // todo: set proper numThreads
			HZ_CORE_INFO("Physics: PxCPUDispacher created");

			// not necessary
			if (!PxInitExtensions(*mPhysics, mPvd))
				HZ_CORE_ASSERT(FALSE, "PxInitExtensions failed!");
			HZ_CORE_INFO("Physics: PxExtensions created");

			HZ_CORE_INFO("Physics: Default material created");
			material = mPhysics->createMaterial(0.1, 0.05, 0.1);
		}

		Physics::~Physics()
		{
			material->release();
			PxCloseExtensions();
			gDispatcher->release();
			mPhysics->release();
			mPvd->release();
			foundation->release();
		}

		Ref<PhysicsScene> Physics::CreateScene()
		{
			HZ_CORE_INFO("Physics: Create new physics scene");
			
			PxTolerancesScale tolerancesScale = mPhysics->getTolerancesScale();
			PxSceneDesc sceneDesc{tolerancesScale};
			sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
			sceneDesc.cpuDispatcher = gDispatcher;
			sceneDesc.filterShader = PxDefaultSimulationFilterShader;
			sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVE_ACTORS;
			
			PxScene* pScene = mPhysics->createScene(sceneDesc);
			auto scene = Ref<PhysicsScene>::Create(pScene);
			return scene;
		}

		const PxMaterial& Physics::GetDefaultMaterial() const
		{
			return *material;
		}
	}

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
