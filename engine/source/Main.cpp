#include "game/Game.h"

#include <iostream>

#include "runtime/RuntimeIface.h"
#include "config/ConfigSystem.h"
#include "config/ConfigVar.h"
#include "logs/Logs.h"

static ConfigVar<int> cfgWindowWidth("/Engine/Window/Width", 800);
static ConfigVar<int> cfgWindowHeight("/Engine/Window/Height", 800);

#ifdef EDITOR
#include "editor/init/EditorConfigurator.h"
#include "editor/runtime/EditorRuntime.h"
using AppConfigurator = EditorConfigurator;
using AppRuntime = EditorRuntime;
#else
#include "init/EngineConfigurator.h"
#include "runtime/EngineRuntime.h"
using AppConfigurator = EngineConfigurator;
using AppRuntime = EngineRuntime;
#endif

#if defined(_WIN32) && defined(WIN_MAIN)

#include <windows.h>

int CALLBACK WinMain(
	HINSTANCE   hInstance,
	HINSTANCE   hPrevInstance,
	LPSTR       lpCmdLine,
	int         nCmdShow
) 
{

#else

// TEMP: use main to show console window
int main(int argc, char* argv[])
{

#endif // _WIN32

	//setlocale(LC_ALL, ""); // TODO: remove when logging is done
	TriadLogs::Init();

	ConfigSystem cfgSys;
	std::unique_ptr<AppRuntime> appRuntime;
	{
		AppConfigurator configurator;
		if (!configurator.CreateRuntime(argc, argv, appRuntime)) {
			return EXIT_FAILURE;
		}
	}
	RuntimeIface::InitParams params;
	params.window.width = cfgWindowWidth;
	params.window.height = cfgWindowHeight;
	if (!appRuntime->Init(params)) {
		return EXIT_FAILURE;
	}
	appRuntime->Run();
	appRuntime->Shutdown();

	return 0;
}
