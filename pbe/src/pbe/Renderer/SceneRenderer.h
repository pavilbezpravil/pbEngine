#pragma once

#include "PipelineState.h"
#include "RootSignature.h"
#include "pbe/Core/Singleton.h"
#include "pbe/Scene/Scene.h"
#include "pbe/Renderer/Mesh.h"
#include "pbe/Renderer/PipelineState.h"
#include "pbe/Scene/SceneCamera.h"


namespace pbe {

	class SceneRenderer : public Singleton<SceneRenderer>
	{
	public:
		struct CameraInfo
		{
			Mat4 viewProj;
		};
		
		void Init();

		void BeginScene(const Ref<Scene>& scene, const CameraInfo& cameraInfo);
		void EndScene();

		void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform = glm::mat4(1.0f));
	private:
		void FlushDrawList();

		Ref<Scene> _scene;
		CameraInfo _cameraInfo;
		
		struct DrawCommand
		{
			Ref<Mesh> mesh;
			glm::mat4 transform;
		};
		std::vector<DrawCommand> _drawList;

		// todo:
		GraphicsPSO pso;
		Ref<Shader> vs;
		Ref<Shader> ps;

		RootSignature BaseRootSignature;

		void InitBaseRootSignature() {
			BaseRootSignature.Reset(2);
			BaseRootSignature[0].InitAsConstantBuffer(0);
			BaseRootSignature[1].InitAsConstantBuffer(1);
			BaseRootSignature.Finalize(L"Base", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		}
	};

}
