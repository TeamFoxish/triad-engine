#include "EngineRuntime.h"

#include "os/Window.h"

#include "shared/SharedStorage.h"

#include "render/RenderInit.h"
#include "render/RenderSystem.h"

#include "resource/ResourceInit.h"
#include "resource/ResourceSystem.h"

#include "init/InitLoader.h"
#include "scene/SceneLoader.h"
#include "scene/Scene.h"

#include "input/InputDevice.h"

#include "game/Game.h"

#include "editor/ui_debug/UIDebug.h"

#include "scripts/ScriptSystem.h"
#include "scripts/ScriptObject.h"

ConfigVar<std::string_view> cfgProjectName("/Project/Name", "DefaultProjectName");
ConfigVar<std::string_view> cfgInitResource("/Project/Resource/InitResource", "res://init.resource");

std::unique_ptr<Game> gTempGame = nullptr;

MulticastDelegate<int, int> gViewportResized;

bool EngineRuntime::Init(const InitParams& params)
{
	if (params.window.createWindow) {
		window = osCreateWindow(cfgProjectName.GetRef().data(), params.window.width, params.window.height);
		if (!window) {
			// TODO: log error message
			return false;
		}
	}

	gTempGame = std::make_unique<Game>();

	// init systems
	if (!InitSharedStorage()) {
		// TODO: log error message
		return false;
	}
	if (!InitRender(this)) {
		// TODO: log error message
		return false;
	}
	if (!InitResource(this)) {
		// TODO: log error message
		return false;
	}
	if(!InitScript(this)) {
		// TODO: log error message
		return false;
	}

	globalInputDevice = new InputDevice(this);

	gResourceSys->LoadResource(ToStrid(cfgInitResource.GetRef().data()));
	gScriptSys->BuildModules();
	ScriptObject* sceneRoot = SceneLoader::CreateScene(InitLoader::startUpSceneTag);
	gScriptSys->SetScene(sceneRoot);
	delete sceneRoot;

	UIDebug::Init(window);

	return true;
}

void EngineRuntime::Run()
{
	bool success = gTempGame->Initialize();
	if (success) {
		while (gTempGame->isRunning) {
			gTempGame->UpdateFrame();

			SharedStorage::Instance().transforms.Update();

			UIDebug::StartNewFrame();
			gRenderSys->StartFrame();

			UIDebug::TestDraw();
			UIDebug::Render();

			gRenderSys->EndFrame();
		}
	}
}

void EngineRuntime::Shutdown()
{
	UIDebug::Destroy();
	gTempGame->Shutdown();
	// delete globalInputDevice; crushes
	TermResource(this);
	TermRender(this);
	TermScript(this);
	TermSharedStorage();
	if (window) {
		osDestroyWindow(window);
	}
}
