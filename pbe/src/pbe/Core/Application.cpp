#include "pch.h"
#include "Application.h"

#include "pbe/Renderer/Renderer.h"
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <filesystem>

#include "pbe/Script/ScriptEngine.h"

#include <Windows.h>

// dx12
#include "Input.h"
#include "pbe/Audio/AudioScene.h"
#include "pbe/Physics/PhysicsScene.h"
#include "pbe/Renderer/CommandContext.h"
#include "pbe/Renderer/GraphicsCore.h"
#include "pbe/Renderer/ColorBuffer.h"


namespace pbe {

#define BIND_EVENT_FN(fn) std::bind(&Application::##fn, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationProps& props)
	{
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(props.Name, props.WindowWidth, props.WindowHeight)));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(true);

		Input::Init();

		Renderer::Instantiate();
		Renderer::Get().Init();

		m_ImGuiLayer = new ImGuiLayer("ImGui");
		PushOverlay(m_ImGuiLayer);

		physics::Init();
		audio::Init();

		ScriptEngine::Init();
	}

	Application::~Application()
	{
		m_LayerStack.Clear();

		audio::Term();
		physics::Term();

		ScriptEngine::Shutdown();
		Renderer::Get().Shutdown();

		Input::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::RenderImGui()
	{
		m_ImGuiLayer->Begin();

		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();

		m_ImGuiLayer->End();
	}

	void Application::Run()
	{
		OnInit();
		
		while (m_Running) {
			// dx12
			{
				GraphicsContext& Context = GraphicsContext::Begin(L"Clear BB");
				Context.ClearColor(Graphics::GetCurrentBB());
				Context.Finish();
			}

			if (!m_Minimized) {
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(m_TimeStep);

				// todo: Render ImGui on render thread
				RenderImGui();
			}
			m_Window->OnUpdate();

			float time = GetTime();
			m_TimeStep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			// debug handler
			if (m_TimeStep.GetSeconds() > 1.f) {
				m_TimeStep = 1.f / 60.f;
			}
		}

		for (auto& l : m_LayerStack) {
			l->OnDetach();
		}

		OnShutdown();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		int width = e.GetWidth(), height = e.GetHeight();	
		if (width == 0 || height == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;

		Graphics::Resize(width, height);

		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	std::string Application::OpenFile(const char* filter) const
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = *(HWND*)m_Window->GetNativeWindowHandler();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE) {
			return std::filesystem::relative(ofn.lpstrFile).string();
		}
		return std::string();
	}

	std::string Application::SaveFile(const char* filter) const
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = *(HWND*)m_Window->GetNativeWindowHandler();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE) {
			return std::filesystem::relative(ofn.lpstrFile).string();
			// return ofn.lpstrFile;
		}
		return std::string();
	}

	float Application::GetTime() const
	{
		return (float)glfwGetTime();
	}

	const char* Application::GetConfigurationName()
	{
#if defined(HZ_DEBUG)
		return "Debug";
#elif defined(HZ_RELEASE)
		return "Release";
#elif defined(HZ_DIST)
		return "Dist";
#else
	#error Undefined configuration?
#endif
	}

	const char* Application::GetPlatformName()
	{
#if defined(HZ_PLATFORM_WINDOWS)
		return "Windows x64";
#else
	#error Undefined platform?
#endif
	}

}
