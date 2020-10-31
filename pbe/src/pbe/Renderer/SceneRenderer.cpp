#include "pch.h"
#include "SceneRenderer.h"

namespace pbe {

	void SceneRenderer::Init()
	{
		
	}

	void SceneRenderer::BeginScene(const Ref<Scene>& scene, const CameraInfo& cameraInfo)
	{
		HZ_CORE_ASSERT(!_scene);
		_scene = scene;
		_cameraInfo = cameraInfo;
	}

	void SceneRenderer::EndScene()
	{
		HZ_CORE_ASSERT(_scene);
		FlushDrawList();
		_scene = nullptr;
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform)
	{
		DrawCommand cmd = {mesh, transform};
		_drawList.push_back(cmd);
	}

	void SceneRenderer::FlushDrawList()
	{
		// todo:
		// dx12
	}

}
