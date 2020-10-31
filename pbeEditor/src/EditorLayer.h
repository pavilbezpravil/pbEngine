#pragma once

#include "pbe.h"

#include "pbe/ImGui/ImGuiLayer.h"
#include "pbe/Editor/EditorCamera.h"
#include "imgui/imgui_internal.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <string>

#include "pbe/Editor/SceneHierarchyPanel.h"

namespace pbe {

	class EditorLayer : public Layer
	{
	public:
		enum class PropertyFlag
		{
			None = 0, ColorProperty = 1, DragProperty = 2, SliderProperty = 4
		};
	public:
		EditorLayer();
		virtual ~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;

		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		// ImGui UI helpers
		bool Property(const std::string& name, bool& value);
		bool Property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		bool Property(const std::string& name, glm::vec2& value, PropertyFlag flags);
		bool Property(const std::string& name, glm::vec2& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		bool Property(const std::string& name, glm::vec3& value, PropertyFlag flags);
		bool Property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);
		bool Property(const std::string& name, glm::vec4& value, PropertyFlag flags);
		bool Property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None);

		void SelectEntity(Entity entity);

		void OpenScene();
		void SaveScene();
		void SaveSceneAs();
	private:
		std::pair<float, float> GetMouseViewportSpace();
		std::pair<glm::vec3, glm::vec3> CastRay(float mx, float my);

		struct SelectedSubmesh
		{
			pbe::Entity Entity;
			Submesh* Mesh = nullptr;
			float Distance = 0.0f;
		};

		void OnSelected(const SelectedSubmesh& selectionContext);
		void OnEntityDeleted(Entity e);
		Ray CastMouseRay();

		void OnScenePlay();
		void OnSceneStop();

		void UpdateWindowTitle(const std::string& sceneName);

		float GetSnapValue();
	private:
		void OnImGuiMenuBar(bool &p_open);
		void OnImGuiRendererInfo();
		void OnImGuiViewport();
		void OnImGuiSceneHierarchy();
		void OnImGuiGizmo();
		
		Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;

		Ref<Scene> m_RuntimeScene, m_EditorScene;
		std::string m_SceneFilePath;
		bool m_ReloadScriptOnPlay = true;

		EditorCamera m_EditorCamera;

		// Editor resources
		Ref<Texture2D> m_PlayButtonTex;

		glm::vec2 m_ViewportBounds[2];
		int m_GizmoType = -1; // -1 = no gizmo
		float m_SnapValue = 0.5f;
		float m_RotationSnapValue = 45.0f;
		bool m_AllowViewportCameraEvents = false;

		bool m_ViewportPanelMouseOver = false;
		bool m_ViewportPanelFocused = false;

		enum class SceneState
		{
			Edit = 0, Play = 1, Pause = 2
		};
		SceneState m_SceneState = SceneState::Edit;

		enum class SelectionMode
		{
			None = 0, Entity = 1, SubMesh = 2
		};

		SelectionMode m_SelectionMode = SelectionMode::Entity;
		std::vector<SelectedSubmesh> m_SelectionContext;
		glm::mat4* m_RelativeTransform = nullptr;
		glm::mat4* m_CurrentlySelectedTransform = nullptr;
	};

}
