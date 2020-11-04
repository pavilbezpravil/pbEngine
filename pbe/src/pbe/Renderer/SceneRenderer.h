#pragma once

#include "DepthBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "pbe/Core/Singleton.h"
#include "pbe/Scene/Scene.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Renderer/DepthBuffer.h"
#include "pbe/Renderer/PipelineState.h"
#include "pbe/Scene/SceneCamera.h"
#include "pbe/Scene/Components.h"


namespace pbe {

	class Renderer;

	struct Light
	{
		enum Type
		{
			Direction,
			Point,
			Spot,
		};

		Vec3 positionOrDirection = { 0.0f, 0.0f, 0.0f };
		float radius;
		Vec3 radiance = { 0.0f, 0.0f, 0.0f };
		float cutOff;
		Type type;

		Vec3 up; // in case spot light is direction

		void InitAsDirectLight(const Vec3& direction, const Vec3& up, const Vec3& radiance)
		{
			type = Light::Direction;
			positionOrDirection = direction;
			this->up = up;
			this->radiance = radiance;
		}

		void InitAsPointLight(const Vec3& position, const Vec3& radiance, float radius)
		{
			type = Light::Point;
			positionOrDirection = position;
			this->radiance = radiance;
			this->radius = radius;
		}

		void InitAsSpotLight(const Vec3& position, const Vec3& direction, const Vec3& radiance, float radius, float cutOff)
		{
			type = Light::Spot;
			positionOrDirection = position;
			this->radiance = radiance;
			this->radius = radius;
			this->cutOff = cutOff;
			up = direction;
		}
	};

	class SceneRenderer : public Singleton<SceneRenderer>
	{
		friend Renderer;

	public:
		struct Environment
		{
			std::vector<Light> lights;
		};

		struct CameraInfo
		{
			Mat4 viewProj;
			Vec3 position;
		};
		
		void Init();
		void Shutdown();

		void BeginScene(const Ref<Scene>& scene, const CameraInfo& cameraInfo, const Environment& environment);
		void EndScene();

		void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f));
	private:
		Mat4 GetShadowViewProj();

		void DrawAllMesh(GraphicsContext& context);
		void ShadowPass();
		void DepthPass();
		void ColorPass();
		void FlushDrawList();

		Ref<Scene> _scene;
		CameraInfo _cameraInfo;
		Environment _environment;
		
		struct DrawCommand
		{
			Ref<Mesh> mesh;
			glm::mat4 transform;
		};
		std::vector<DrawCommand> _drawList;

		Ref<Shader> vs;
		Ref<Shader> ps;

		Ref<Shader> vs_shadow;
		Ref<Shader> ps_shadow;

		Ref<DepthBuffer> _shadowBuffer;

		StructuredBuffer _lightsGPUBuffer;

		Ref<RootSignature> BaseRootSignature = nullptr;

		void InitBaseRootSignature();
	};

}
