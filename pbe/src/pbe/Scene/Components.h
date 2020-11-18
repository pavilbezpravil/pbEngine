#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>


#include "pbe/Core/UUID.h"
#include "pbe/Renderer/Texture.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Scene/SceneCamera.h"

namespace pbe {


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

	struct TransformComponent
	{
		Vec3 Translation = {0, 0, 0};
		Quat Rotation = glm::quat(1, 0, 0, 0);
		Vec3 Scale = {1, 1, 1};

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		TransformComponent(const Vec3& translation)
			: Translation(translation) {}

		Vec3 Forward() const { return Rotation * Vec3_X; }
		Vec3 Up() const { return Rotation * Vec3_Y; }
		Vec3 Right() const { return Rotation * Vec3_Z; }

		Mat4 GetTransform() const;
		void SetTransform(const Mat4& trans);

		operator glm::mat4 () const { return GetTransform(); }

		COMPONENT_CLASS_TYPE(TransformComponent)
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
		bool Primary = true;

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
