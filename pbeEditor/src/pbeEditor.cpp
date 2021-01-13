#include <pbe.h>
#include <pbe/EntryPoint.h>

#include "EditorLayer.h"

class pbeEditorApplication : public pbe::Application
{
public:
	pbeEditorApplication(const pbe::ApplicationProps& props)
		: Application(props)
	{
	}

	virtual void OnInit() override
	{
		PushLayer(new pbe::EditorLayer());
	}
};

pbe::Application* pbe::CreateApplication(int argc, char** argv)
{
	return new pbeEditorApplication({"pbeEditor", 1600, 900});
}
