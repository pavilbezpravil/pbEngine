#pragma once

#include "RendObj.h"
#include "DepthBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "pbe/Core/Singleton.h"
#include "pbe/Scene/Scene.h"


namespace pbe {

	class Renderer;

	class SceneRenderer : public Singleton<SceneRenderer>
	{
		friend Renderer;

	public:
		struct Environment
		{
			std::vector<RendLight> lights;
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
