#pragma once

#include "pbe.h"
#include <string>

namespace pbe {

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer(const std::string& scenePath);

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;

		virtual void OnEvent(Event& e) override;
	private:
		Ref<Scene> m_RuntimeScene;
		std::string scenePath;
	};
}
