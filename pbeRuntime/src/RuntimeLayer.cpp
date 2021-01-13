#include "RuntimeLayer.h"

#include "pbe/Renderer/GraphicsCore.h"


namespace pbe {

	RuntimeLayer::RuntimeLayer(const std::string& scenePath) : scenePath(scenePath)
	{
	}

	void RuntimeLayer::OnAttach()
	{
		auto scene = Ref<Scene>::Create();
		SceneSerializer serializer(scene);
		if (serializer.Deserialize(scenePath)) {
			m_RuntimeScene = scene;
			m_RuntimeScene->OnRuntimeStart();
			std::string title = scenePath + " - pbeRuntime - " + Application::GetPlatformName() + " (" + Application::GetConfigurationName() + ")";
			Application::Get().GetWindow().SetTitle(title);
		} else {
			Application::Get().OnEvent(WindowCloseEvent{});
		}
	}

	void RuntimeLayer::OnDetach()
	{
		if (!m_RuntimeScene) {
			return;
		}
		
		m_RuntimeScene->OnRuntimeStop();
		m_RuntimeScene = nullptr;
	}

	void RuntimeLayer::OnEvent(Event& e)
	{
		if (!m_RuntimeScene) {
			return;
		}

		m_RuntimeScene->OnEvent(e);
	}

	void RuntimeLayer::OnUpdate(Timestep ts)
	{
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
