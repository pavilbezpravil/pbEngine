#pragma once

#include "pbe/Core/Singleton.h"
#include "pbe/Scene/Scene.h"
#include "pbe/Renderer/Mesh.h"
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
	};

}
