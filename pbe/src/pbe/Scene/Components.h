#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>


#include "pbe/Core/UUID.h"
#include "pbe/Renderer/Texture.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Scene/SceneCamera.h"

namespace pbe {

	class Scene;

#define COMPONENT_CLASS_TYPE(ComponentType) static const char* GetName() { return STRINGIFY(ComponentType); }

	struct IDComponent
	{
		UUID ID = 0;

		COMPONENT_CLASS_TYPE(IDComponent)
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent& other) = default;
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

		void Attach(UUID uuid);
		void Dettach();

		void UpdateChilds() const;

		bool HasParent() const;
		bool HasChilds() const;

		Vec3 HierPosition = Vec3_Zero;
		Quat HierRotation = glm::quat(1, 0, 0, 0);
		Vec3 HierScale = Vec3_One;

		Vec3 LocalPosition = Vec3_Zero;
		Quat LocalRotation = glm::quat(1, 0, 0, 0);
		Vec3 LocalScale = Vec3_One;

		void UpdatePosition(const Vec3& position, Space space = Space::Local);
		void UpdateRotation(const Quat& rotation, Space space = Space::Local);
		void UpdateScale(const Vec3& scale, Space space = Space::Local);

		Vec3 Position(Space space = Space::Local) const;
		Quat Rotation(Space space = Space::Local) const;
		Vec3 Scale(Space space = Space::Local) const;

		Vec3 WorldPosition() const { return Position(Space::World); }
		Quat WorldRotation() const { return Rotation(Space::World); }
		Vec3 WorldScale() const { return Scale(Space::World); }

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		// TransformComponent(const Vec3& translation)
		// 	: Translation(translation) {}
		// TransformComponent(const Quat& rotation)
		// 	: Rotation(rotation) {}

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
	};

	struct MeshComponent
	{
		Ref<pbe::Mesh> Mesh;

		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;
		MeshComponent(const Ref<pbe::Mesh>& mesh)
			: Mesh(mesh) {}

		operator Ref<pbe::Mesh> () { return Mesh; }

		COMPONENT_CLASS_TYPE(MeshComponent)
	};

	struct ScriptComponent
	{
		std::string ScriptPath;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent& other) = default;
		ScriptComponent(const std::string& moduleName)
			: ScriptPath(moduleName) {}

		COMPONENT_CLASS_TYPE(ScriptComponent)
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other) = default;

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
		DirectionLightComponent() = default;
		DirectionLightComponent(const DirectionLightComponent& other) = default;

		COMPONENT_CLASS_TYPE(DirectionLightComponent)
	};

	struct PointLightComponent : LightComponentBase
	{
		float Radius = 10.0f;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent& other) = default;

		COMPONENT_CLASS_TYPE(PointLightComponent)
	};

	struct SpotLightComponent : LightComponentBase
	{
		float Radius = 10.0f;
		float CutOff = 45.0f;

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent& other) = default;

		COMPONENT_CLASS_TYPE(SpotLightComponent)
	};

}
