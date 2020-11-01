#pragma once

#include "PipelineState.h"
#include "RootSignature.h"
#include "pbe/Core/Singleton.h"
#include "pbe/Scene/Scene.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Renderer/PipelineState.h"
#include "pbe/Scene/SceneCamera.h"
#include "pbe/Scene/Components.h"


namespace pbe {
	struct DirectionLight
	{
		DirectionLightComponent directionLightComponent;
		Vec3 Direction;
	};

	class SceneRenderer : public Singleton<SceneRenderer>
	{
	public:
		struct Environment
		{
			DirectionLight directionLight;
		};

		struct CameraInfo
		{
			Mat4 viewProj;
			Vec3 position;
		};
		
		void Init();

		void BeginScene(const Ref<Scene>& scene, const CameraInfo& cameraInfo, const Environment& environment);
		void EndScene();

		void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f));
	private:
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

		Ref<RootSignature> BaseRootSignature = nullptr;

		void InitBaseRootSignature();
	};

}
