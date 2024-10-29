#include "EngineRuntime.h"

#include "os/Window.h"

#include "render/RenderInit.h"
#include "render/RenderSystem.h"

#include "input/InputDevice.h"

#include "game/Game.h"

ConfigVar<std::string_view> cfgProjectName("/Project/Name", "DefaultProjectName");

bool EngineRuntime::Init(const InitParams& params)
{
	if (params.window.createWindow) {
		window = osCreateWindow(cfgProjectName.GetRef().data(), params.window.width, params.window.height);
		if (!window) {
			// TODO: log error message
			return false;
		}
	}

	if (!InitRender(this)) {
		// TODO: log error message
		return false;
	}
	
	globalInputDevice = new InputDevice(this);

	return true;
}

void EngineRuntime::Run()
{
	std::unique_ptr<Game> tempGame = std::make_unique<Game>();
	bool success = tempGame->Initialize();
	if (success) {
		while (tempGame->isRunning) {
			tempGame->UpdateFrame();
			gRenderSys->DrawFrame();
		}
	}
	tempGame->Shutdown();
}

void EngineRuntime::Shutdown()
{
	TermRender(this);
	if (window) {
		osDestroyWindow(window);
	}
}
