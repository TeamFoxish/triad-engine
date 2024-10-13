#include "core/Game.h"

#include <iostream>

//#define TEMP_CFG_VAR_TEST
#ifdef TEMP_CFG_VAR_TEST

#include "config/ConfigVar.h"
#include "config/ConfigSystem.h"

static ConfigVar<int> cfgVarInt("/Engine/Version", 1);
static ConfigVar<float> cfgVarFloat("/Project/FloatTest", 1.0f);
static ConfigVar<bool> cfgVarBool("/Project/BoolTest", false);
static ConfigVar<const char*> cfgVarEngineName("/Engine/EngineName", "DefaultEngineName");
static ConfigVar<const char*> cfgVarProjectName("/Project/ProjectName", "DefaultProjectName");
static ConfigVar<std::vector<const char*>> cfgVarNames("/Project/CoolNames", {});
static ConfigVar<std::vector<int>> cfgVarNumbers("/Project/CoolNumbers", {});

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
	ConfigSystem cfgSys("config/project.cfg");
	std::cout << cfgVarInt << '\n';
	std::cout << cfgVarEngineName << '\n';
	std::cout << cfgVarProjectName << '\n';
	std::cout << cfgVarFloat << '\n';
	std::cout << cfgVarBool << '\n';
	std::cout << cfgVarNames.GetRef()[0] << '\n';
	std::cout << cfgVarNumbers.GetRef()[0] << '\n';
#endif

	Game game;
	bool success = game.Initialize("TriadEngine", 800, 800);
	if (success)
	{
		game.RunLoop();
	}
	game.Shutdown();

	return 0;
}
