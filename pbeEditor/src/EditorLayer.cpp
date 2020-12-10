#include "EditorLayer.h"

#include "pbe/ImGui/ImGuizmo.h"
#include "pbe/Script/ScriptEngine.h"

#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "pbe/Allocator/Allocator.h"
#include "pbe/Renderer/ColorBuffer.h"
#include "pbe/Renderer/CommandContext.h"
#include "pbe/Renderer/GraphicsCore.h"


namespace pbe {

	static void ImGuiShowHelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	EditorLayer::EditorLayer()
		: m_EditorCamera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f))
	{
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		// ImGui Colors
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f); // Window background
		colors[ImGuiCol_ChildBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.5f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f); // Widget backgrounds
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.4f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 0.6f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.0f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
		colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.4f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.0f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.0f);
		colors[ImGuiCol_Header] = ImVec4(0.7f, 0.7f, 0.7f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.7f, 0.7f, 0.7f, 0.8f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.5f, 0.52f, 1.0f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.5f, 0.5f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.0f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.43f, 0.35f, 1.0f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.6f, 0.15f, 1.0f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.6f, 0.6f, 1.0f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);

		using namespace glm;

		// Editor
		m_PlayButtonTex = Texture2D::Create("assets/editor/PlayButton.png");

		m_EditorScene = Ref<Scene>::Create();
		UpdateWindowTitle("Untitled Scene");
		m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>(m_EditorScene);
		m_SceneHierarchyPanel->SetSelectionChangedCallback(std::bind(&EditorLayer::SelectEntity, this, std::placeholders::_1));
		m_SceneHierarchyPanel->SetEntityDeletedCallback(std::bind(&EditorLayer::OnEntityDeleted, this, std::placeholders::_1));

		// todo: add custom scene deserialize on start
		m_SceneFilePath = "assets/scenes/TestScene.pbsc";
		SceneSerializer serializer(m_EditorScene);
		serializer.Deserialize(m_SceneFilePath);
	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::OnScenePlay()
	{
		m_SelectionContext.clear();

		m_SceneState = SceneState::Play;

		// if (m_ReloadScriptOnPlay)
		// 	s_ScriptEngine->InitState();

		m_RuntimeScene = Ref<Scene>::Create();
		m_EditorScene->CopyTo(m_RuntimeScene);

		m_RuntimeScene->OnRuntimeStart();
		m_SceneHierarchyPanel->SetContext(m_RuntimeScene);
	}

	void EditorLayer::OnSceneStop()
	{
		m_RuntimeScene->OnRuntimeStop();
		m_SceneState = SceneState::Edit;

		// Unload runtime scene
		m_RuntimeScene = nullptr;

		m_SelectionContext.clear();
		m_SceneHierarchyPanel->SetContext(m_EditorScene);
	}

	void EditorLayer::UpdateWindowTitle(const std::string& sceneName)
	{
		std::string title = sceneName + " - pbeEditor - " + Application::GetPlatformName() + " (" + Application::GetConfigurationName() + ")";
		Application::Get().GetWindow().SetTitle(title);
	}

	float EditorLayer::GetSnapValue()
	{
		switch (m_GizmoType)
		{
			case  ImGuizmo::OPERATION::TRANSLATE: return 0.5f;
			case  ImGuizmo::OPERATION::ROTATE: return 45.0f;
			case  ImGuizmo::OPERATION::SCALE: return 0.5f;
		}
		return 0.0f;
	}

	void EditorLayer::OnImGuiMenuBar(bool &p_open) {
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Scene", "Ctrl-N")) {
					// TODO: save current scene dialog
					NewScene();
				}
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
					OpenScene();
				ImGui::Separator();
				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveScene();
				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::Separator();
				if (ImGui::MenuItem("Exit"))
					p_open = false;
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void EditorLayer::OnImGuiRendererInfo() {
		Renderer::Get().OnImGui();
	}

	void EditorLayer::OnImGuiAllocatorInfo()
	{
		ImGui::Begin("Allocator");
		if (ImGui::TreeNodeEx("Total alloc")){
			ImGui::Text("count: %ld", pbeAllocator.TotalAllocCount());
			ImGui::Text("bytes: %ld Kb", pbeAllocator.TotalAllocBytes() >> 10);
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx("Cur alloc", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("count: %ld", pbeAllocator.CurAllocCount());
			ImGui::Text("bytes: %ld Kb", pbeAllocator.CurAllocBytes() >> 10);
			ImGui::TreePop();
		}
		ImGui::End();
	}

	void EditorLayer::OnImGuiViewport() {
		ImGui::Begin("Viewport");

		ImGui::SetNextItemWidth(30);
		switch (m_SceneState) {
			case SceneState::Edit:
				if (ImGui::Button("Play")) {
					OnScenePlay();
				}
				break;
			case SceneState::Play:
				if (ImGui::Button("Pause")) {
					m_SceneState = SceneState::Pause;
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop")) {
					OnSceneStop();
				}
				break;
			case SceneState::Pause:
				if (ImGui::Button("Continue")) {
					m_SceneState = SceneState::Play;
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop")) {
					OnSceneStop();
				}
				break;
			default: HZ_CORE_ASSERT(FALSE);
		}
		ImGui::SameLine();

		ImGui::SetNextItemWidth(80);
		ImGui::Combo("Translate mode", (int*)&m_GizmoTransSpace, "Local\0World\0");
		m_SceneHierarchyPanel->SetTransformSpace(m_GizmoTransSpace);

		m_ViewportPanelMouseOver = ImGui::IsWindowHovered();
		m_ViewportPanelFocused = ImGui::IsWindowFocused();

		ImGuiIO& io = ImGui::GetIO();
		// io.WantCaptureMouse = !m_ViewportPanelFocused || !m_ViewportPanelMouseOver;
		// io.WantCaptureKeyboard = !m_ViewportPanelFocused || !m_ViewportPanelMouseOver;
		io.WantCaptureMouse = !m_ViewportPanelMouseOver;
		io.WantCaptureKeyboard = !m_ViewportPanelMouseOver;

		// HZ_CORE_INFO("viewport hovered {}", m_ViewportPanelMouseOver);
		// HZ_CORE_INFO("viewport focused {}", m_ViewportPanelFocused);
		// HZ_CORE_INFO("io.WantCaptureMouse {}", io.WantCaptureMouse);
		// HZ_CORE_INFO("io.WantCaptureKeyboard {}", io.WantCaptureKeyboard);
		// HZ_CORE_INFO("io.MousePos ({}, {})", io.MousePos.x, io.MousePos.y);

		auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
		auto viewportSize = ImGui::GetContentRegionAvail();

		Renderer::Get().Resize((uint)viewportSize.x, (uint)viewportSize.y);
		m_EditorScene->SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		if (m_RuntimeScene)
			m_RuntimeScene->SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		m_EditorCamera.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), viewportSize.x, viewportSize.y, 0.1f, 1000.0f));
		m_EditorCamera.SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		ImGui::Image(pbeImGui::ImageDesc(Renderer::Get().GetFullScreenColor()->GetSRV()), viewportSize);

		auto windowSize = ImGui::GetWindowSize();
		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
		m_ViewportBounds[0] = { minBound.x, minBound.y };
		m_ViewportBounds[1] = { maxBound.x, maxBound.y };
		m_AllowViewportCameraEvents = ImGui::IsMouseHoveringRect(minBound, maxBound);

		// todo:
		if (m_RuntimeScene) {
			Entity cameraEntity = m_RuntimeScene->GetMainCameraEntity();
			if (cameraEntity) {
				CameraComponent& camera = cameraEntity.GetComponent<CameraComponent>();
				camera.Camera.SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
			}
		}

		OnImGuiGizmo();

		ImGui::End();
	}

	void EditorLayer::OnImGuiSceneHierarchy() {
		m_SceneHierarchyPanel->OnImGuiRender();
	}

	void EditorLayer::OnImGuiGizmo() {
		if (m_GizmoType != -1 && m_SelectionContext.size()) {
			auto& selection = m_SelectionContext[0];

			float rw = (float)ImGui::GetWindowWidth();
			float rh = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh);

			bool snap = Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL);

			Mat4 entityTransform = selection.Entity.GetComponent<TransformComponent>().GetWorldTransform();
			float snapValue = GetSnapValue();
			float snapValues[3] = { snapValue, snapValue, snapValue };
			if (m_SelectionMode == SelectionMode::Entity) {
				ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetViewMatrix()),
					glm::value_ptr(m_EditorCamera.GetProjectionMatrix()),
					(ImGuizmo::OPERATION)m_GizmoType,
					(ImGuizmo::MODE)m_GizmoTransSpace,
					glm::value_ptr(entityTransform),
					nullptr,
					snap ? snapValues : nullptr);

				selection.Entity.GetComponent<TransformComponent>().SetWorldTransform(entityTransform);
			} else {
				glm::mat4 transformBase = entityTransform * selection.Mesh->Transform;
				ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetViewMatrix()),
					glm::value_ptr(m_EditorCamera.GetProjectionMatrix()),
					(ImGuizmo::OPERATION)m_GizmoType,
					(ImGuizmo::MODE)m_GizmoTransSpace,
					glm::value_ptr(transformBase),
					nullptr,
					snap ? snapValues : nullptr);

				selection.Mesh->Transform = glm::inverse(entityTransform) * transformBase;
			}
		}
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				// if (m_ViewportPanelFocused)
					m_EditorCamera.OnUpdate(ts);

				// todo: tmp for test script
				// m_EditorScene->OnUpdate(ts);

				m_EditorScene->OnRenderEditor(m_EditorCamera);

				break;
			}
			case SceneState::Play:
			{
				// if (m_ViewportPanelFocused)
				// 	m_EditorCamera.OnUpdate(ts);

				m_RuntimeScene->OnUpdate(ts);
				m_RuntimeScene->OnRenderRuntime();
				break;
			}
			case SceneState::Pause:
			{
				if (m_ViewportPanelFocused)
					m_EditorCamera.OnUpdate(ts);

				m_RuntimeScene->OnRenderRuntime();
				break;
			}
		}
	}

	bool EditorLayer::Property(const std::string& name, bool& value)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		bool result = ImGui::Checkbox(id.c_str(), &value);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		
		return result;
	}

	bool EditorLayer::Property(const std::string& name, float& value, float min, float max, PropertyFlag flags)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		bool changed = false;
		if (flags == PropertyFlag::SliderProperty)
			changed = ImGui::SliderFloat(id.c_str(), &value, min, max);
		else
			changed = ImGui::DragFloat(id.c_str(), &value, 1.0f, min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		
		return changed;
	}

	bool EditorLayer::Property(const std::string& name, glm::vec2& value, EditorLayer::PropertyFlag flags)
	{
		return Property(name, value, -1.0f, 1.0f, flags);
	}

	bool EditorLayer::Property(const std::string& name, glm::vec2& value, float min, float max, PropertyFlag flags)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		bool changed = false;
		if (flags == PropertyFlag::SliderProperty)
			changed = ImGui::SliderFloat2(id.c_str(), glm::value_ptr(value), min, max);
		else
			changed = ImGui::DragFloat2(id.c_str(), glm::value_ptr(value), 1.0f, min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return changed;
	}

	bool EditorLayer::Property(const std::string& name, glm::vec3& value, EditorLayer::PropertyFlag flags)
	{
		return Property(name, value, -1.0f, 1.0f, flags);
	}

	bool EditorLayer::Property(const std::string& name, glm::vec3& value, float min, float max, EditorLayer::PropertyFlag flags)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		bool changed = false;
		if ((int)flags & (int)PropertyFlag::ColorProperty)
			changed = ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
		else if (flags == PropertyFlag::SliderProperty)
			changed = ImGui::SliderFloat3(id.c_str(), glm::value_ptr(value), min, max);
		else
			changed = ImGui::DragFloat3(id.c_str(), glm::value_ptr(value), 1.0f, min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return changed;
	}

	bool EditorLayer::Property(const std::string& name, glm::vec4& value, EditorLayer::PropertyFlag flags)
	{
		return Property(name, value, -1.0f, 1.0f, flags);
	}

	bool EditorLayer::Property(const std::string& name, glm::vec4& value, float min, float max, EditorLayer::PropertyFlag flags)
	{
		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string id = "##" + name;
		bool changed = false;
		if ((int)flags & (int)PropertyFlag::ColorProperty)
			changed = ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
		else if (flags == PropertyFlag::SliderProperty)
			changed = ImGui::SliderFloat4(id.c_str(), glm::value_ptr(value), min, max);
		else
			changed = ImGui::DragFloat4(id.c_str(), glm::value_ptr(value), 1.0f, min, max);

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return changed;
	}

	void EditorLayer::SelectEntity(Entity entity)
	{
		SelectedSubmesh selection;
		if (entity.HasComponent<MeshComponent>())
		{
			selection.Mesh = &entity.GetComponent<MeshComponent>().Mesh->GetSubmeshes()[0];
		}
		selection.Entity = entity;
		m_SelectionContext.clear();
		m_SelectionContext.push_back(selection);

		m_EditorScene->SetSelectedEntity(entity);
	}

	void EditorLayer::NewScene()
	{
		auto& app = Application::Get();
		std::string filepath = app.OpenFile("pbe Scene (*.pbsc)\0*.pbsc\0");
		if (!filepath.empty())
		{
			Ref<Scene> newScene = Ref<Scene>::Create();
			m_EditorScene = newScene;
			std::filesystem::path path = filepath;
			UpdateWindowTitle(path.filename().string());
			m_SceneHierarchyPanel->SetContext(m_EditorScene);

			m_EditorScene->SetSelectedEntity({});
			m_SelectionContext.clear();

			m_SceneFilePath = filepath;
		}
	}
	
	void EditorLayer::OpenScene()
	{
		auto& app = Application::Get();
		std::string filepath = app.OpenFile("pbe Scene (*.pbsc)\0*.pbsc\0");
		if (!filepath.empty())
		{
			Ref<Scene> newScene = Ref<Scene>::Create();
			SceneSerializer serializer(newScene);
			serializer.Deserialize(filepath);
			m_EditorScene = newScene;
			std::filesystem::path path = filepath;
			UpdateWindowTitle(path.filename().string());
			m_SceneHierarchyPanel->SetContext(m_EditorScene);

			m_EditorScene->SetSelectedEntity({});
			m_SelectionContext.clear();

			m_SceneFilePath = filepath;
		}
	}

	void EditorLayer::SaveScene()
	{
		SceneSerializer serializer(m_EditorScene);
		serializer.Serialize(m_SceneFilePath);
	}

	void EditorLayer::SaveSceneAs()
	{
		auto& app = Application::Get();
		std::string filepath = app.SaveFile("pbe Scene (*.pbsc)\0*.pbsc\0");
		if (!filepath.empty())
		{
			SceneSerializer serializer(m_EditorScene);
			serializer.Serialize(filepath);

			std::filesystem::path path = filepath;
			UpdateWindowTitle(path.filename().string());
			m_SceneFilePath = filepath;
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		static bool p_open = true;

		static bool opt_fullscreen_persistant = true;
		static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
		bool opt_fullscreen = opt_fullscreen_persistant;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		//if (opt_flags & ImGuiDockNodeFlags_PassthruDockspace)
		//	window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &p_open, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Dockspace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
		}

		OnImGuiMenuBar(p_open);
		OnImGuiSceneHierarchy();
		OnImGuiRendererInfo();
		OnImGuiAllocatorInfo();
		OnImGuiViewport();
		s_ScriptEngine->OnImGuiRender();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		if (m_SceneState == SceneState::Edit)
		{
			if (m_ViewportPanelMouseOver)
				m_EditorCamera.OnEvent(e);

			m_EditorScene->OnEvent(e);
		}
		else if (m_SceneState == SceneState::Play)
		{
			m_RuntimeScene->OnEvent(e);
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		if (m_ViewportPanelFocused || m_ViewportPanelMouseOver)
		{
			switch (e.GetKeyCode())
			{
				case KeyCode::Q:
					m_GizmoType = -1;
					break;
				case KeyCode::W:
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
					break;
				case KeyCode::E:
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
					break;
				case KeyCode::R:
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
					break;
				case KeyCode::Escape:
					if (!m_SelectionContext.empty()) {
						m_SelectionContext.clear();
						m_EditorScene->SetSelectedEntity({});
						m_SceneHierarchyPanel->SetSelected({});
					}
					break;
				case KeyCode::Delete:
					if (!m_SelectionContext.empty()) {
						Entity selectedEntity = m_SelectionContext[0].Entity;
						m_EditorScene->DestroyEntity(selectedEntity);
						m_SelectionContext.clear();
						m_EditorScene->SetSelectedEntity({});
						m_SceneHierarchyPanel->SetSelected({});
					}
					break;
			}
		}

		if (Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL))
		{
			switch (e.GetKeyCode())
			{
				case KeyCode::D:
					if (m_SelectionContext.size()) {
						Entity selectedEntity = m_SelectionContext[0].Entity;
						m_EditorScene->DuplicateEntity(selectedEntity);
					}
					break;
				case KeyCode::N:
					NewScene();
					break;
				case KeyCode::O:
					OpenScene();
					break;
				case KeyCode::S:
					SaveScene();
					break;
				case KeyCode::R:
					Shader::Recompile(true);
					break;
			}

			if (Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT))
			{
				switch (e.GetKeyCode())
				{
				case KeyCode::S:
					SaveSceneAs();
					break;
				}
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		auto [mx, my] = Input::GetMousePosition();
		if (e.GetMouseButton() == HZ_MOUSE_BUTTON_LEFT && !Input::IsKeyPressed(KeyCode::LeftAlt) && !ImGuizmo::IsOver() && m_SceneState != SceneState::Play)
		{
			auto [mouseX, mouseY] = GetMouseViewportSpace();
			if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
			{
				auto [origin, direction] = CastRay(mouseX, mouseY);

				m_SelectionContext.clear();
				m_EditorScene->SetSelectedEntity({});
				auto meshEntities = m_EditorScene->GetAllEntitiesWith<MeshComponent>();
				for (auto e : meshEntities)
				{
					Entity entity = { e, m_EditorScene.Raw() };
					auto mesh = entity.GetComponent<MeshComponent>().Mesh;
					if (!mesh)
						continue;

					auto& submeshes = mesh->GetSubmeshes();
					float lastT = std::numeric_limits<float>::max();
					for (uint32_t i = 0; i < submeshes.size(); i++) {
						auto& submesh = submeshes[i];
						Ray ray = {
							glm::inverse(entity.GetComponent<TransformComponent>().GetWorldTransform() * submesh.Transform) * glm::vec4(origin, 1.0f),
							glm::inverse(glm::mat3(entity.GetComponent<TransformComponent>().GetWorldTransform()) * glm::mat3(submesh.Transform)) * direction
						};

						float t;
						bool intersects = ray.IntersectsAABB(submesh.BoundingBox, t);
						if (intersects) {
							m_SelectionContext.push_back({ entity, &submesh, t });
						}
					}
				}
				std::sort(m_SelectionContext.begin(), m_SelectionContext.end(), [](auto& a, auto& b) { return a.Distance < b.Distance; });
				if (m_SelectionContext.size())
					OnSelected(m_SelectionContext[0]);

			}
		}
		return false;
	}

	std::pair<float, float> EditorLayer::GetMouseViewportSpace()
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
		auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;

		return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
	}

	std::pair<glm::vec3, glm::vec3> EditorLayer::CastRay(float mx, float my)
	{
		glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

		auto inverseProj = glm::inverse(m_EditorCamera.GetProjectionMatrix());
		auto inverseView = glm::inverse(glm::mat3(m_EditorCamera.GetViewMatrix()));

		glm::vec4 ray = inverseProj * mouseClipPos;
		glm::vec3 rayPos = m_EditorCamera.GetPosition();
		glm::vec3 rayDir = inverseView * glm::vec3(ray);

		return { rayPos, rayDir };
	}

	void EditorLayer::OnSelected(const SelectedSubmesh& selectionContext)
	{
		m_SceneHierarchyPanel->SetSelected(selectionContext.Entity);
		m_EditorScene->SetSelectedEntity(selectionContext.Entity);
	}

	void EditorLayer::OnEntityDeleted(Entity e)
	{
		if (m_SelectionContext[0].Entity == e)
		{
			m_SelectionContext.clear();
			m_EditorScene->SetSelectedEntity({});
		}
	}

	Ray EditorLayer::CastMouseRay()
	{
		auto [mouseX, mouseY] = GetMouseViewportSpace();
		if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
		{
			auto [origin, direction] = CastRay(mouseX, mouseY);
			return Ray(origin, direction);
		}
		return Ray::Zero();
	}

}
