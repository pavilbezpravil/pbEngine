#include "pch.h"
#include "StandartComponents.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Entity.h"
#include "Scene.h"
#include "pbe/Core/Utility.h"
#include "pbe/Physics/PhysXTypeConvet.h"

namespace pbe
{
	void TransformComponent::Attach(UUID uuid)
	{
		HZ_CORE_ASSERT(ownUUID != uuid);
		HZ_CORE_ASSERT(uuid != UUID_INVALID);
		DettachFromParent();
		// if (uuid == UUID_INVALID) {
		// 	return;
		// }

		Vec3 position = Position(Space::World);
		Quat rotation = Rotation(Space::World);
		Vec3 scale = Scale(Space::World);

		Entity parentEntity = pScene->GetEntityMap().at(uuid);
		auto& parentTrans = parentEntity.GetComponent<TransformComponent>();

		ParentUUID = uuid;
		parentTrans.ChildUUIDs.push_back(ownUUID);

		Hier.Position = parentTrans.WorldPosition();
		Hier.Rotation = parentTrans.WorldRotation();
		Hier.Scale = parentTrans.WorldScale();

		UpdatePosition(position, Space::World);
		UpdateRotation(rotation, Space::World);
		UpdateScale(scale, Space::World);
	}

	void TransformComponent::DettachFromParent()
	{
		if (!HasParent()) {
			return;
		}

		Vec3 position = Position(Space::World);
		Quat rotation = Rotation(Space::World);
		Vec3 scale = Scale(Space::World);

		Entity parentEntity = pScene->GetEntityMap().at(ParentUUID);
		auto& parentTrans = parentEntity.GetComponent<TransformComponent>();

		int pos = vector_find(parentTrans.ChildUUIDs, ownUUID);
		HZ_CORE_ASSERT(pos != -1);
		vector_fast_erase(parentTrans.ChildUUIDs, pos);

		ParentUUID = UUID_INVALID;

		UpdatePosition(position, Space::World);
		UpdateRotation(rotation, Space::World);
		UpdateScale(scale, Space::World);
	}

	void TransformComponent::DettachChilds()
	{
		if (!HasChilds()) {
			return;
		}

		std::vector<UUID> childs = ChildUUIDs;
		for (UUID uuid : childs) {
			Entity e = pScene->GetEntityMap().at(uuid);
			auto& trans = e.GetComponent<TransformComponent>();

			trans.DettachFromParent();
		}
	}

	void TransformComponent::UpdateChilds() const
	{
		if (!HasChilds()) {
			return;
		}

		Vec3 totalPosition = WorldPosition();
		Quat totalRotation = WorldRotation();
		Vec3 totalScale = WorldScale();

		for (UUID uuid : ChildUUIDs) {
			Entity e = pScene->GetEntityMap().at(uuid);
			auto& trans = e.GetComponent<TransformComponent>();

			trans.Hier.Position = totalPosition;
			trans.Hier.Rotation = totalRotation;
			trans.Hier.Scale = totalScale;

			trans.NotifyTransformChanged();
			trans.UpdateChilds();
		}
	}

	bool TransformComponent::HasParent() const
	{
		return ParentUUID != UUID_INVALID;
	}

	bool TransformComponent::HasChilds() const
	{
		return !ChildUUIDs.empty();
	}

	void TransformComponent::UpdatePosition(const Vec3& position, Space space)
	{
		if (!HasParent() || space == Space::Local) {
			Local.Position = position;
		} else {
			Quat invHierRotation = glm::inverse(Hier.Rotation);
			Local.Position = invHierRotation * (position - Hier.Position) / Hier.Scale;
		}
		UpdateChilds();
		NotifyTransformChanged();
	}

	void TransformComponent::UpdateRotation(const Quat& rotation, Space space)
	{
		if (!HasParent() || space == Space::Local) {
			Local.Rotation = rotation;
		} else {
			Quat invHierRotation = glm::inverse(Hier.Rotation);
			Local.Rotation = invHierRotation * rotation;
		}
		UpdateChilds();
		NotifyTransformChanged();
	}

	void TransformComponent::UpdateScale(const Vec3& scale, Space space)
	{
		if (!HasParent() || space == Space::Local) {
			Local.Scale = scale;
		} else {
			Local.Scale = scale / Hier.Scale;
		}
		UpdateChilds();
		NotifyTransformChanged();
	}

	Vec3 TransformComponent::Position(Space space) const
	{
		if (!HasParent() || space == Space::Local) {
			return Local.Position;
		} else {
			return Hier.Position + Hier.Rotation * (Local.Position * Hier.Scale);
		}
	}

	Quat TransformComponent::Rotation(Space space) const
	{
		if (!HasParent() || space == Space::Local) {
			return Local.Rotation;
		} else {
			return Hier.Rotation * Local.Rotation;
		}
	}

	Vec3 TransformComponent::Scale(Space space) const
	{
		if (!HasParent() || space == Space::Local) {
			return Local.Scale;
		} else {
			return Hier.Scale * Local.Scale;
		}
	}

	void TransformComponent::Move(const Vec3& move, Space space)
	{
		UpdatePosition(Position(space) + move, space);
	}

	void TransformComponent::Rotate(const Quat& rotation, Space space)
	{
		UpdateRotation(rotation * Rotation(space), space);
	}

	Transform TransformComponent::GetTransform(Space space) const
	{
		if (space == Space::Local) {
			return Local;
		} else {
			return Transform(Position(space), Rotation(space), Scale(space));
		}
	}

	void TransformComponent::SetTransform(const Transform& trans, Space space)
	{
		if (space == Space::Local) {
			Local = trans;
		} else {
			UpdatePosition(trans.Position, space);
			UpdateRotation(trans.Rotation, space);
			UpdateScale(trans.Scale, space);
		}
	}

	void TransformComponent::NotifyTransformChanged()
	{
		// if (OnTransformChanged) {
		// 	OnTransformChanged();
		// }

		// todo: tmp solution
		Entity e = pScene->GetEntityMap().at(ownUUID);
		pScene->GetPhysicsScene()->OnEntityTransformChanged(e);
	}

	void ColliderComponentBase::UpdateIsTrigger()
	{
		_shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, !IsTrigger);
		_shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, IsTrigger);
	}

	void ColliderComponentBase::UpdateCenter()
	{
		_shape->setLocalPose(PxTransform(Vec3ToPx(Center)));
	}

	void ColliderComponentBase::UpdateAllBase()
	{
		UpdateIsTrigger();
		UpdateCenter();
	}

	void SphereColliderComponent::UpdateRadius()
	{
		PxSphereGeometry sphereGeom;
		bool success = _shape->getSphereGeometry(sphereGeom);
		HZ_CORE_ASSERT(success);
		sphereGeom.radius = Radius;
		_shape->setGeometry(sphereGeom);
	}

	void SphereColliderComponent::UpdateAll()
	{
		UpdateAllBase();
		UpdateRadius();
	}

	void BoxColliderComponent::UpdateSize()
	{
		PxBoxGeometry boxGeom;
		bool success = _shape->getBoxGeometry(boxGeom);
		HZ_CORE_ASSERT(success);
		boxGeom.halfExtents = Vec3ToPx(Size) * 0.5;
		_shape->setGeometry(boxGeom);
	}

	void BoxColliderComponent::UpdateAll()
	{
		UpdateAllBase();
		UpdateSize();
	}

	void RigidbodyComponent::UpdateMass()
	{
		_actor->setMass(Mass);
	}

	void RigidbodyComponent::UpdateDrag()
	{
		_actor->setLinearDamping(Drag);
	}

	void RigidbodyComponent::UpdateAngularDrag()
	{
		_actor->setLinearDamping(AngularDrag);
	}

	void RigidbodyComponent::UpdateUseGravity()
	{
		_actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !UseGravity);
	}

	void RigidbodyComponent::UpdateIsKinematic()
	{
		_actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, IsKinematic);
	}

	void RigidbodyComponent::UpdateAll()
	{
		UpdateMass();
		UpdateDrag();
		UpdateAngularDrag();
		
		UpdateUseGravity();
		UpdateIsKinematic();
	}

	Vec3 RigidbodyComponent::GetVelocity() const
	{
		return PxVec3ToPBE(_actor->getLinearVelocity());
	}

	void RigidbodyComponent::SetVelocity(Vec3 v)
	{
		_actor->setLinearVelocity(Vec3ToPx(v));
	}

	Vec3 RigidbodyComponent::GetAngularVelocity() const
	{
		return PxVec3ToPBE(_actor->getAngularVelocity());
	}

	void RigidbodyComponent::SetAngularVelocity(Vec3 v)
	{
		_actor->setAngularVelocity(Vec3ToPx(v));
	}

	void RigidbodyComponent::AddForce(Vec3 f)
	{
		_actor->addForce(Vec3ToPx(f));
	}

	void RigidbodyComponent::SetupFiltering(uint32_t filterGroup, uint32_t filterMask)
	{
		PxFilterData filterData;
		filterData.word0 = filterGroup; // word0 = own ID
		filterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a
										// contact callback;
		const PxU32 numShapes = _actor->getNbShapes();
		PxShape** shapes = new PxShape*[numShapes]; // todo:
		_actor->getShapes(shapes, numShapes);
		for (PxU32 i = 0; i < numShapes; i++) {
			PxShape* shape = shapes[i];
			shape->setSimulationFilterData(filterData);
		}
		delete[](shapes);
	}
}
