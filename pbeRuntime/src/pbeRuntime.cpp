#include <pbe.h>
#include <pbe/EntryPoint.h>

#include "RuntimeLayer.h"

class pbeRuntimeApplication : public pbe::Application
{
public:
	pbeRuntimeApplication(const pbe::ApplicationProps& props, const std::string& scenePath)
		: Application(props), scenePath(scenePath)
	{
	}

	virtual void OnInit() override
	{
		PushLayer(new pbe::RuntimeLayer(scenePath));
	}

private:
	std::string scenePath;
};

pbe::Application* pbe::CreateApplication(int argc, char** argv)
{
	// return new pbeRuntimeApplication({ "pbeRuntime", 1600, 900 }, "C:/Users/Pavel/Documents/dev/pbEngine/bin/builds/pbe_0_0_2/assets/scenes/TestScene.pbsc");
	if (argc == 2) {
		return new pbeRuntimeApplication({ "pbeRuntime", 1600, 900 }, argv[1]);
	}
	return NULL;
}
