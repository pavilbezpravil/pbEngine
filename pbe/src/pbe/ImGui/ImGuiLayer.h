#pragma once

#include "pch.h"

#include "pbe/Core/Layer.h"

namespace pbe {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		ImGuiLayer(const std::string& name);
		virtual ~ImGuiLayer();

		void OnEvent(Event& event) override;

		void Begin();
		void End();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
	private:
		float m_Time = 0.0f;
	};

	namespace pbeImGui {
		void* ImageDesc(D3D12_CPU_DESCRIPTOR_HANDLE Handle);
	}

}
