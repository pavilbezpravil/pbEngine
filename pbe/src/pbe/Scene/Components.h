#pragma once

#include <glm/glm.hpp>

#include "pbe/Core/UUID.h"
#include "pbe/Renderer/Texture.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Scene/SceneCamera.h"

namespace pbe {

	struct IDComponent
	{
		UUID ID = 0;
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
	};

	struct TransformComponent
	{
		glm::mat4 Transform;

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		TransformComponent(const glm::mat4& transform)
			: Transform(transform) {}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }
	};

	struct MeshComponent
	{
		Ref<pbe::Mesh> Mesh;

		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;
		MeshComponent(const Ref<pbe::Mesh>& mesh)
			: Mesh(mesh) {}

		operator Ref<pbe::Mesh> () { return Mesh; }
	};

	struct ScriptComponent
	{
		std::string ModuleName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent& other) = default;
		ScriptComponent(const std::string& moduleName)
			: ModuleName(moduleName) {}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other) = default;

		operator SceneCamera& () { return Camera; }
		operator const SceneCamera& () const { return Camera; }
	};

	struct LightComponentBase
	{
		bool Enable = true;
		bool CastShadow = true;
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
	};

	struct PointLightComponent : LightComponentBase
	{
		float Radius = 10.0f;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent& other) = default;
	};

	struct SpotLightComponent : LightComponentBase
	{
		float Radius = 10.0f;
		float CutOff = 45.0f;

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent& other) = default;
	};

}
