#pragma once


#include "DepthBuffer.h"
#include "Mesh.h"

#include "pbe/Core/Singleton.h"

class ColorBuffer;

namespace pbe {

	class Renderer : public Singleton<Renderer>
	{
	public:
		void Init();
		void Shutdown();

		void Resize(uint width, uint height);

		Ref<ColorBuffer>& GetFullScreenColor() { return g_fullScreenColor; }
		Ref<DepthBuffer>& GetFullScreenDepth() { return g_fullScreenDepth; }

		void OnImGui();

	private:
		Ref<ColorBuffer> g_fullScreenColor;
		Ref<DepthBuffer> g_fullScreenDepth;
		uint g_width = 0;
		uint g_height = 0;
		DXGI_FORMAT g_fullScrennColorFormat;
		DXGI_FORMAT g_fullScrennDepthFormat;
	};

}
