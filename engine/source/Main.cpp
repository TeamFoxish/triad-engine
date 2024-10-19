#include "core/Game.h"

#include <iostream>

#include "runtime/RuntimeIface.h"
#include "config/ConfigSystem.h"

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

//#define TEMP_CFG_VAR_TEST
#ifdef TEMP_CFG_VAR_TEST

#include "config/ConfigVar.h"
#include "config/ConfigSystem.h"

static ConfigVar<std::string_view> cfgVarEngineName("/Engine/Name", "DefaultEngineName");
static ConfigVar<std::string_view> cfgVarProjectName("/Project/Name", "DefaultProjectName");
static ConfigVar<std::vector<std::string_view>> cfgVarNames("/Engine/CoolNames", {});
static ConfigVar<std::vector<int>> cfgVarNumbers("/Engine/ArrayTest", {});

#endif // TEMP_CFG_VAR_TEST

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

#ifdef TEMP_CFG_VAR_TEST
	setlocale(LC_ALL, "");
	ConfigSystem cfgSys;
	cfgSys.Init("config/engine.cfg");
	cfgSys.Override("config/project.cfg");
	std::cout << cfgVarEngineName.GetRef() << '\n';
	std::cout << cfgVarProjectName.GetRef() << '\n';
	std::cout << cfgVarNames.GetRef()[0] << '\n';
	std::cout << cfgVarNumbers.GetRef()[0] << '\n';
	return 0;
#else

	setlocale(LC_ALL, ""); // TODO: remove when logging is done
	ConfigSystem cfgSys;
	std::unique_ptr<AppRuntime> appRuntime;
	{
		AppConfigurator configurator;
		if (!configurator.CreateRuntime(argc, argv, appRuntime)) {
			return EXIT_FAILURE;
		}
	}

	Game game;
	bool success = game.Initialize("TriadEngine", 800, 800);
	if (success)
	{
		game.RunLoop();
	}
	game.Shutdown();

	return 0;
#endif
}
