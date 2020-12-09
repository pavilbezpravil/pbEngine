#pragma once

#include "pbe/Scene/Scene.h"
#include "pbe/Scene/Entity.h"
#include "pbe/Renderer/Mesh.h"

namespace pbe {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);
		void SetTransformSpace(Space space);
		void SetSelected(Entity entity);
		void SetSelectionChangedCallback(const std::function<void(Entity)>& func) { m_SelectionChangedCallback = func; }
		void SetEntityDeletedCallback(const std::function<void(Entity)>& func) { m_EntityDeletedCallback = func; }

		void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
		void DrawMeshNode(const Ref<Mesh>& mesh, uint32_t& imguiMeshID);
		void MeshNodeHierarchy(const Ref<Mesh>& mesh, aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
		Space m_TransSpace = Space::Local;

		bool m_WasTransAttach = false;
		struct TransAttachInfo
		{
			UUID attached;
			UUID parent;
		} m_TransAttachInfo;

		std::function<void(Entity)> m_SelectionChangedCallback, m_EntityDeletedCallback;
	};

}
