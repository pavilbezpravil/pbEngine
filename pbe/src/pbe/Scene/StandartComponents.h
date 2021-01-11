#pragma once

#include "pbe/Scene/Components.h"
#include "pbe/Core/UUID.h"
#include "pbe/Core/Math/Transform.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Scene/SceneCamera.h"
#include "pbe/AI/AIController.h"


namespace pbe {

	class Scene;

	struct TagComponent
	{
		std::string Tag;

		TagComponent(const std::string& tag)
			: Tag(tag) {}

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }

		COMPONENT_CLASS_TYPE(TagComponent)
	};

	// must be equal ImGuiGizmo
	enum class Space {
		Local = 0,
		World = 1,
	};
	
	struct TransformComponent
	{
		UUID ownUUID = UUID_INVALID;
		Scene* pScene = NULL;

		UUID ParentUUID = UUID_INVALID;
		std::vector<UUID> ChildUUIDs;

		Transform Hier;
		Transform Local;

		// std::function<void()> OnTransformChanged = NULL;
		
		void Attach(UUID uuid);
		void DettachFromParent();
		void DettachChilds();

		void UpdateChilds() const;

		bool HasParent() const;
		bool HasChilds() const;

		void UpdatePosition(const Vec3& position, Space space = Space::Local);
		void UpdateRotation(const Quat& rotation, Space space = Space::Local);
		void UpdateScale(const Vec3& scale, Space space = Space::Local);

		Vec3 Position(Space space = Space::Local) const;
		Quat Rotation(Space space = Space::Local) const;
		Vec3 Scale(Space space = Space::Local) const;

		Vec3 WorldPosition() const { return Position(Space::World); }
		Quat WorldRotation() const { return Rotation(Space::World); }
		Vec3 WorldScale() const { return Scale(Space::World); }

		Vec3 LocalForward() const { return Forward(Space::Local); }
		Vec3 LocalUp() const { return Up(Space::Local); }
		Vec3 LocalRight() const { return Right(Space::Local); }

		Vec3 WorldForward() const { return Forward(Space::World); }
		Vec3 WorldUp() const { return Up(Space::World); }
		Vec3 WorldRight() const { return Right(Space::World); }

		Transform GetLocalTransform() const { return GetTransform(Space::Local); }
		void SetLocalTransform(const Transform& trans) { return SetTransform(trans, Space::Local); }

		Transform GetWorldTransform() const { return GetTransform(Space::World); }
		void SetWorldTransform(const Transform& trans) { return SetTransform(trans, Space::World); }

		void Move(const Vec3& move, Space space = Space::Local);
		void Rotate(const Quat& rotation, Space space = Space::Local);
		
		// operator glm::mat4 () const { return GetTransform(); }

		COMPONENT_CLASS_TYPE(TransformComponent)

	private:
		Vec3 Forward(Space space = Space::Local) const { return Rotation(space) * Vec3_ZNeg; }
		Vec3 Up(Space space = Space::Local) const { return Rotation(space) * Vec3_Y; }
		Vec3 Right(Space space = Space::Local) const { return Rotation(space) * Vec3_X; }

		Transform GetTransform(Space space = Space::Local) const;
		void SetTransform(const Transform& trans, Space space = Space::Local);
		
		void NotifyTransformChanged();
	};

	struct MeshComponent
	{
		Ref<pbe::Mesh> Mesh;

		MeshComponent(const Ref<pbe::Mesh>& mesh)
			: Mesh(mesh) {}

		operator Ref<pbe::Mesh> () { return Mesh; }

		COMPONENT_CLASS_TYPE(MeshComponent)
	};

	struct ScriptComponent
	{
		std::string ScriptPath;

		ScriptComponent(const std::string& moduleName)
			: ScriptPath(moduleName) {}

		COMPONENT_CLASS_TYPE(ScriptComponent)
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = false;

		operator SceneCamera& () { return Camera; }
		operator const SceneCamera& () const { return Camera; }

		COMPONENT_CLASS_TYPE(CameraComponent)
	};

	struct LightComponentBase
	{
		bool Enable = true;
		bool CastShadow = false;
		Vec3 Color = { 1.0f, 1.0f, 1.0f };
		float Multiplier = 1.0f;

	protected:
		LightComponentBase() = default;
		LightComponentBase(const LightComponentBase& other) = default;
	};
	
	struct DirectionLightComponent : LightComponentBase
	{
		COMPONENT_CLASS_TYPE(DirectionLightComponent)
	};

	struct PointLightComponent : LightComponentBase
	{
		float Radius = 10.0f;

		COMPONENT_CLASS_TYPE(PointLightComponent)
	};

	struct SpotLightComponent : LightComponentBase
	{
		float Radius = 10.0f;
		float CutOff = 45.0f;

		COMPONENT_CLASS_TYPE(SpotLightComponent)
	};

	struct ColliderComponentBase
	{
		bool IsTrigger = false;
		Vec3 Center = Vec3_Zero;

		physx::PxShape* _shape = NULL; // internal

		void UpdateCenter();
		
	protected:
		ColliderComponentBase() = default;
		ColliderComponentBase(const ColliderComponentBase& other) = default;
	};

	struct SphereColliderComponent : ColliderComponentBase
	{
		float Radius = 0.5f;

		void UpdateRadius();

		COMPONENT_CLASS_TYPE(SphereColliderComponent)
	};

	// struct PlaneColliderComponent : ColliderComponentBase
	// {
	// 	Vec3 Center = Vec3_Zero;
	// 	float Radius = 0.5f;
	//
	// 	COMPONENT_CLASS_TYPE(PlaneColliderComponent)
	// };
	
	struct BoxColliderComponent : ColliderComponentBase
	{
		Vec3 Size = Vec3_One;

		void UpdateSize();
		
		COMPONENT_CLASS_TYPE(BoxColliderComponent)
	};

	struct RigidbodyComponent
	{
		float Mass = 1.f;
		float Drag = 0.02f;
		float AngularDrag = 0.05f;
		
		bool UseGravity = true;
		bool IsKinematic = false;

		void UpdateMass();
		void UpdateDrag();
		void UpdateAngularDrag();

		void UpdateUseGravity();
		void UpdateIsKinematic();

		void UpdateAll();

		Vec3 GetVelocity() const;
		void SetVelocity(Vec3 v);

		Vec3 GetAngularVelocity() const;
		void SetAngularVelocity(Vec3 v);
		
		physx::PxRigidDynamic* _actor = NULL; // internal
		bool _pandingForDestroy = false; // internal

		COMPONENT_CLASS_TYPE(RigidbodyComponent)
	};

	struct AIControllerComponent
	{
		Ref<AI::Controller> AIController;

		COMPONENT_CLASS_TYPE(AIControllerComponent)
	};
	
}