#pragma once

#ifdef HZ_PLATFORM_WINDOWS

extern pbe::Application* pbe::CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
	pbe::InitializeCore();
	pbe::Application* app = pbe::CreateApplication(argc, argv);
	HZ_CORE_ASSERT(app, "Client Application is null!");
	app->Run();
	delete app;
	pbe::ShutdownCore();
}

#endif
