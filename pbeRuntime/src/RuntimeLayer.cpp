#include "RuntimeLayer.h"

#include "pbe/Renderer/GraphicsCore.h"


namespace pbe {

	RuntimeLayer::RuntimeLayer(const std::string& scenePath) : scenePath(scenePath)
	{
	}

	void RuntimeLayer::OnAttach()
	{
		LoadRuntimeScene();
	}

	void RuntimeLayer::OnDetach()
	{
		UnloadRuntimeScene();
	}

	void RuntimeLayer::OnEvent(Event& e)
	{
		// EventDispatcher d(e);
		// d.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) {
		// 	if (e.GetMouseButton() == HZ_MOUSE_BUTTON_LEFT) {
		// 		Application::Get().GetWindow().SetMouseMode(MouseMode::Disabled);
		// 		return true;
		// 	}
		// 	return false;
		// });
		// d.Dispatch<WindowLostFocusEvent>([&](WindowLostFocusEvent&) {
		// 	Application::Get().GetWindow().SetMouseMode(MouseMode::Normal);
		// 	return false;
		// });
		
		if (!m_RuntimeScene || e.Handled) {
			return;
		}

		m_RuntimeScene->OnEvent(e);
	}

	bool RuntimeLayer::LoadRuntimeScene()
	{
		auto scene = Ref<Scene>::Create();
		SceneSerializer serializer(scene);
		if (serializer.Deserialize(scenePath)) {
			m_RuntimeScene = scene;
			m_RuntimeScene->OnRuntimeStart();
			std::string title = scenePath + " - pbeRuntime - " + Application::GetPlatformName() + " (" + Application::GetConfigurationName() + ")";
			Application::Get().GetWindow().SetTitle(title);
			Application::Get().GetWindow().SetMouseMode(MouseMode::Disabled);
			return true;
		} else {
			Application::Get().OnEvent(WindowCloseEvent{});
			return false;
		}
	}

	void RuntimeLayer::UnloadRuntimeScene()
	{
		if (!m_RuntimeScene) {
			return;
		}

		m_RuntimeScene->OnRuntimeStop();
		m_RuntimeScene = nullptr;
	}

	void RuntimeLayer::OnUpdate(Timestep ts)
	{
		if (m_RuntimeScene && m_RuntimeScene->IsReloadRequested()) {
			UnloadRuntimeScene();
			LoadRuntimeScene();
		}
		
		if (!m_RuntimeScene) {
			return;
		}
		
		const auto& window = Application::Get().GetWindow();
		m_RuntimeScene->SetViewportSize(window.GetWidth(), window.GetHeight());
		Renderer::Get().Resize(window.GetWidth(), window.GetHeight());
		
		m_RuntimeScene->OnUpdate(ts);
		m_RuntimeScene->OnRenderRuntime(RTSet{ Graphics::GetCurrentBB(), Renderer::Get().GetFullScreenDepth() });
		m_RuntimeScene->OnNextFrame();
	}

}
