#include "pch.h"
#include "Components.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Entity.h"
#include "Scene.h"
#include "pbe/Core/Utility.h"

namespace pbe
{
	void TransformComponent::Attach(UUID uuid)
	{
		HZ_CORE_ASSERT(ownUUID != uuid);
		Dettach();

		Vec3 position = Position(Space::World);
		Quat rotation = Rotation(Space::World);
		Vec3 scale = Scale(Space::World);

		Entity parentEntity = pScene->GetEntityMap().at(uuid);
		auto& parentTrans = parentEntity.GetComponent<TransformComponent>();

		ParentUUID = uuid;
		parentTrans.ChildUUIDs.push_back(ownUUID);

		UpdatePosition(position, Space::World);
		UpdateRotation(rotation, Space::World);
		UpdateScale(scale, Space::World);
	}

	void TransformComponent::Dettach()
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

			trans.HierPosition = totalPosition;
			trans.HierRotation = totalRotation;
			trans.HierScale = totalScale;

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
			LocalPosition = position;
		} else {
			Quat invHierRotation = glm::inverse(HierRotation);
			LocalPosition = invHierRotation * (position - HierPosition) / HierScale;
		}
		UpdateChilds();
	}

	void TransformComponent::UpdateRotation(const Quat& rotation, Space space)
	{
		if (!HasParent() || space == Space::Local) {
			LocalRotation = rotation;
		} else {
			Quat invHierRotation = glm::inverse(HierRotation);
			LocalRotation = invHierRotation * rotation;
		}
		UpdateChilds();
	}

	void TransformComponent::UpdateScale(const Vec3& scale, Space space)
	{
		if (!HasParent() || space == Space::Local) {
			LocalScale = scale;
		} else {
			LocalScale = scale / HierScale;
		}
		UpdateChilds();
	}

	Vec3 TransformComponent::Position(Space space) const
	{
		if (!HasParent() || space == Space::Local) {
			return LocalPosition;
		} else {
			return HierPosition + HierRotation * (LocalPosition * HierScale);
		}
	}

	Quat TransformComponent::Rotation(Space space) const
	{
		if (!HasParent() || space == Space::Local) {
			return LocalRotation;
		} else {
			return HierRotation * LocalRotation;
		}
	}

	Vec3 TransformComponent::Scale(Space space) const
	{
		if (!HasParent() || space == Space::Local) {
			return LocalScale;
		} else {
			return HierScale * LocalScale;
		}
	}

	Mat4 TransformComponent::GetTransform(Space space) const
	{
		Mat4 rotation = glm::toMat4(Rotation(space));

		return glm::translate(glm::mat4(1.0), Position(space))
			* rotation
			* glm::scale(glm::mat4(1.0), Scale(space));
	}

	void TransformComponent::SetTransform(const Mat4& trans, Space space)
	{
		auto [position, rotation, scale] = GetTransformDecomposition(trans);

		UpdatePosition(position, space);
		UpdateRotation(rotation, space);
		UpdateScale(scale, space);
	}

}
