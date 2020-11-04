#include "pch.h"
#include "ImGuiLayer.h"

#include "imgui.h"
#include "ImGuizmo.h"

// #define IMGUI_IMPL_API

#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx12.h"

#include "pbe/Core/Application.h"
#include "pbe/Core/Events/MouseEvent.h"
#include "pbe/Core/Events/KeyEvent.h"
#include "pbe/Renderer/CommandContext.h"
#include "pbe/Renderer/ColorBuffer.h"
#include "pbe/Renderer/DescriptorHeap.h"
#include "pbe/Renderer/DynamicDescriptorHeap.h"
#include "pbe/Renderer/GraphicsCore.h"

#include "pbe/Renderer/Renderer.h"

// dx12
extern DXGI_FORMAT SwapChainFormat;

#define NUM_FRAMES_IN_FLIGHT 3

namespace pbe {

	namespace pbeImGui {
		namespace {
			const uint DESC_SIZE = 32;
			UserDescriptorHeap m_descHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, DESC_SIZE };
			std::vector<DescriptorHandle> m_allocatedDescHandles;
			uint m_nextAllocHandleIdx = 1;
		}

		ImTextureID ImageDesc(D3D12_CPU_DESCRIPTOR_HANDLE Handle) {
			uint idx = m_nextAllocHandleIdx;
			m_nextAllocHandleIdx++;
			if (m_nextAllocHandleIdx >= DESC_SIZE) {
				m_nextAllocHandleIdx = 1;
			}
			Graphics::g_Device->CopyDescriptorsSimple(1, m_allocatedDescHandles[idx].GetCpuHandle(), Handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			return reinterpret_cast<ImTextureID>(m_allocatedDescHandles[idx].GetGpuHandle().ptr);
		}
	}

	ImGuiLayer::ImGuiLayer()
	{

	}

	ImGuiLayer::ImGuiLayer(const std::string& name)
	{

	}

	ImGuiLayer::~ImGuiLayer()
	{

	}

	void ImGuiLayer::OnEvent(Event& event) {
		EventDispatcher d(event);
		d.Dispatch<MouseButtonPressedEvent>([](MouseButtonPressedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[e.GetMouseButton()] = true;
			return false;
		});
		d.Dispatch<MouseButtonReleasedEvent>([](MouseButtonReleasedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[e.GetMouseButton()] = false;
			return false;
		});

		d.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.KeysDown[(int)e.GetKeyCode()] = true;
			return false;
		});
		
		d.Dispatch<KeyReleasedEvent>([](KeyReleasedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.KeysDown[(int)e.GetKeyCode()] = false;
			return false;
		});

		d.Dispatch<KeyTypedEvent>([](KeyTypedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddInputCharacter((uint)e.GetKeyCode());
			return false;
		});

		d.Dispatch<MouseScrolledEvent>([](MouseScrolledEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheel = e.GetYOffset();
			io.MouseWheelH = e.GetXOffset();
			return false;
		});
	}

	void ImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		ImFont* pFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
		io.FontDefault = io.Fonts->Fonts.back();

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		pbeImGui::m_descHeap.Create(L"ImGui Descriptor heap");

		pbeImGui::m_allocatedDescHandles.resize(pbeImGui::DESC_SIZE);
		pbeImGui::m_allocatedDescHandles[0] = pbeImGui::m_descHeap.Alloc(1);
		for (int i = 1; i < pbeImGui::DESC_SIZE; ++i) {
			pbeImGui::m_allocatedDescHandles[i] = pbeImGui::m_descHeap.Alloc(1);
		}

		ID3D12DescriptorHeap* srvDescHeap = pbeImGui::m_descHeap.GetHeapPointer();
		auto fontHandle = pbeImGui::m_descHeap.GetHandleAtOffset(0);

		Application& app = Application::Get();
		HWND hWnd = *static_cast<const HWND*>(app.GetWindow().GetNativeWindowHandler());
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX12_Init(Graphics::g_Device, NUM_FRAMES_IN_FLIGHT,
			SwapChainFormat,  srvDescHeap,
			fontHandle.GetCpuHandle(),
			fontHandle.GetGpuHandle());

		SetupKeyMap();
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		pbeImGui::m_descHeap.DestroyAll();
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		GraphicsContext& context = GraphicsContext::Begin(L"ImGui");
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, pbeImGui::m_descHeap.GetHeapPointer());
		context.SetRenderTarget(Graphics::GetCurrentBB());

		// Rendering
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(NULL, (void*)context.GetCommandList());
		}

		context.Finish();
	}

	void ImGuiLayer::OnImGuiRender()
	{
	}

	void ImGuiLayer::SetupKeyMap()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.KeyMap[ImGuiKey_Space] = (int)HZ_KEY_SPACE;
		io.KeyMap[ImGuiKey_Backspace] = (int)HZ_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Escape] = (int)HZ_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_Delete] = (int)HZ_KEY_DELETE;
		io.KeyMap[ImGuiKey_LeftArrow] = (int)HZ_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = (int)HZ_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = (int)HZ_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = (int)HZ_KEY_DOWN;
		io.KeyMap[ImGuiKey_Enter] = (int)HZ_KEY_ENTER;
		io.KeyMap[ImGuiKey_Tab] = (int)HZ_KEY_TAB;
		io.KeyMap[ImGuiKeyModFlags_Alt] = (int)HZ_KEY_LEFT_ALT;
		io.KeyMap[ImGuiKeyModFlags_Ctrl] = (int)HZ_KEY_LEFT_CONTROL;
		io.KeyMap[ImGuiKeyModFlags_Shift] = (int)HZ_KEY_LEFT_SHIFT;
		io.KeyMap[ImGuiKeyModFlags_Super] = (int)HZ_KEY_LEFT_SUPER;
	}
}
