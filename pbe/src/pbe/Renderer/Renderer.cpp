#include "pch.h"
#include "Renderer.h"

#include "Shader.h"

#include "SceneRenderer.h"
#include "ColorBuffer.h"
#include "GraphicsCore.h"
#include "pbe/Core/Application.h"

// dx12
extern DXGI_FORMAT SwapChainFormat;

namespace pbe {

	Ref<ColorBuffer> g_finalRT;
	uint g_width = 0;
	uint g_height = 0;
	DXGI_FORMAT g_finalRTFormat;
	
	void Renderer::Init()
	{
		auto& window = Application::Get().GetWindow();
		Graphics::Initialize(*(HWND*)window.GetNativeWindowHandler(), window.GetWidth(), window.GetHeight());

		g_finalRTFormat = SwapChainFormat;
		g_finalRT = Ref<ColorBuffer>(new ColorBuffer);

		Renderer::Resize(window.GetWidth(), window.GetHeight());

		SceneRenderer::Instantiate();
		SceneRenderer::Get().Init();
	}

	void Renderer::Shutdown() {
		Graphics::Shutdown();
	}

	void Renderer::Resize(uint width, uint height) {
		if (g_width == width || g_height == height) {
			return;
		}
		g_width = width;
		g_height = height;
		g_finalRT->Create(L"Final RT", g_width, g_height, 1, g_finalRTFormat);
	}

	Ref<ColorBuffer>& Renderer::GetFinalRT() {
		return g_finalRT;
	}

}
