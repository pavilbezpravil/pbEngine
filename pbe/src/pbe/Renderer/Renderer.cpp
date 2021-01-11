#include "pch.h"
#include "Renderer.h"

#include <imgui/imgui.h>
#include "Shader.h"
#include "CommandContext.h"

#include "SceneRenderer.h"
#include "ColorBuffer.h"
#include "GraphicsCore.h"
#include "pbe/Core/Application.h"

// dx12
extern DXGI_FORMAT SwapChainFormat;

namespace pbe {

	void Renderer::Init()
	{
		auto& window = Application::Get().GetWindow();
		Graphics::Initialize(*(HWND*)window.GetNativeWindowHandler(), window.GetWidth(), window.GetHeight());

		g_fullScrennColorFormat = SwapChainFormat;
		g_fullScrennDepthFormat = DXGI_FORMAT_D32_FLOAT;
		g_fullScreenColor = Ref<ColorBuffer>::Create();
		g_fullScreenDepth = Ref<DepthBuffer>::Create(1.0f, 0);

		Renderer::Resize(window.GetWidth(), window.GetHeight());

		SceneRenderer::Instantiate();
		SceneRenderer::Get().Init();
	}

	void Renderer::Shutdown() {
		SceneRenderer::Get().Shutdown();
		SceneRenderer::Deinstantiate();
		Shader::Deinit();
		g_fullScreenColor = nullptr;
		g_fullScreenDepth = nullptr;
		Graphics::Shutdown();
	}

	void Renderer::Resize(uint width, uint height) {
		if (g_width == width || g_height == height) {
			return;
		}
		g_width = width;
		g_height = height;
		g_fullScreenColor->Create(L"Full Screen Color", g_width, g_height, 1, g_fullScrennColorFormat);
		g_fullScreenDepth->Create(L"Full Screen Depth", g_width, g_height, 1, g_fullScrennDepthFormat);
	}

	void Renderer::OnImGui()
	{
		ImGui::Text("Frame Time: %.2fms\n", Application::Get().GetTimeStep().GetMilliseconds());

		if (ImGui::TreeNodeEx("ContextManager")){
			if (ImGui::TreeNodeEx("Available context")) {
				for (int i = 0; i < 4; ++i) {
					ImGui::Text("Type %d: %d", i, Graphics::g_ContextManager.sm_AvailableContexts[i].size());
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNodeEx("Context pool")) {
				for (int i = 0; i < 4; ++i) {
					ImGui::Text("Type %d: %d", i, Graphics::g_ContextManager.sm_ContextPool[i].size());
				}
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Descriptor allocator")) {
			for (int i = 0; i < 4; ++i) {
				ImGui::Text("Type %d: %d", i, Graphics::g_DescriptorAllocator[i].sm_TotalAllocatedHandlers);
			}
			ImGui::TreePop();
		}

		// ImGui::Begin("Shadow Buffer");
		// ImGui::Image(pbeImGui::ImageDesc(SceneRenderer::Get()._shadowBuffer->GetDepthSRV()), ImGui::GetContentRegionAvail());
		// ImGui::End();
	}
}
