#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>


#include "pbe/Core/UUID.h"
#include "pbe/Renderer/Texture.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Scene/SceneCamera.h"

namespace pbe {

	class Scene;

#define COMPONENT_CLASS_TYPE(ComponentType) \
	ComponentType() = default;\
	ComponentType(const ComponentType& other) = default; \
	static const char* GetName() { return STRINGIFY(ComponentType); }

	struct IDComponent
	{
		UUID ID;

		COMPONENT_CLASS_TYPE(IDComponent)
	};

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

		Vec3 HierPosition = Vec3_Zero;
		Quat HierRotation = glm::quat(1, 0, 0, 0);
		Vec3 HierScale = Vec3_One;

		Vec3 LocalPosition = Vec3_Zero;
		Quat LocalRotation = glm::quat(1, 0, 0, 0);
		Vec3 LocalScale = Vec3_One;

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

		Mat4 GetLocalTransform() const { return GetTransform(Space::Local); }
		void SetLocalTransform(const Mat4& trans) { return SetTransform(trans, Space::Local); }

		Mat4 GetWorldTransform() const { return GetTransform(Space::World); }
		void SetWorldTransform(const Mat4& trans) { return SetTransform(trans, Space::World); }

		operator glm::mat4 () const { return GetTransform(); }

		COMPONENT_CLASS_TYPE(TransformComponent)

	private:
		Vec3 Forward(Space space = Space::Local) const { return Rotation(space) * Vec3_ZNeg; }
		Vec3 Up(Space space = Space::Local) const { return Rotation(space) * Vec3_Y; }
		Vec3 Right(Space space = Space::Local) const { return Rotation(space) * Vec3_X; }

		Mat4 GetTransform(Space space = Space::Local) const;
		void SetTransform(const Mat4& trans, Space space = Space::Local);
		
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

		physx::PxRigidActor* _actor = NULL; // internal
		bool _pandingForDestroy = false; // internal

		COMPONENT_CLASS_TYPE(RigidbodyComponent)
	};
	
}
