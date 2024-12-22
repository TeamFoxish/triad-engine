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
#include "logs/Logs.h"


ConfigVar<std::string_view> cfgProjectName("/Project/Name", "DefaultProjectName");
ConfigVar<std::string_view> cfgInitResource("/Project/Resource/InitResource", "res://init.resource");

std::unique_ptr<Game> gTempGame = nullptr;

MulticastDelegate<int, int> gViewportResized;

bool EngineRuntime::Init(const InitParams& params)
{
	if (!gEngineRuntime) {
		gEngineRuntime = this;
	}
	if (params.window.createWindow) {
		window = osCreateWindow(cfgProjectName.GetRef().data(), params.window.width, params.window.height);
		if (!window) {
			// TODO: change message(?)
			LOG_ERROR("Failed on window creation");
			return false;
		}
	}

	gTempGame = std::make_unique<Game>();

	// init systems
	if (!InitSharedStorage()) {
		// TODO: change message(?)
		LOG_ERROR("Failed on SharedStorage Initialization");
		return false;
	}
	if (!InitRender(this)) {
		// TODO: change message(?)
		LOG_ERROR("Failed on SharedStorage Initialization");
		return false;
	}
	if (!InitResource(this)) {
		// TODO: change message(?)
		LOG_ERROR("Failed on Reaource Initialization");
		return false;
	}
	extern void InitSceneTree();
	InitSceneTree();
	if(!InitScript(this)) {
		// TODO: change message(?)
		LOG_ERROR("Failed on Script Initialization");
		return false;
	}

	globalInputDevice = new InputDevice(this);

	gResourceSys->LoadResource(ToStrid(cfgInitResource.GetRef().data()));
	gScriptSys->BuildModules();
	std::unique_ptr<ScriptObject> sceneRoot = SceneLoader::CreateScene(InitLoader::startUpSceneTag);
	gScriptSys->SetScene(sceneRoot.get());

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
	TermScript(this);
	extern void TermSceneTree();
	TermSceneTree();
	TermResource(this);
	TermRender(this);
	TermSharedStorage();
	if (window) {
		osDestroyWindow(window);
	}
}

Math::Vector2 EngineRuntime::GetMousePosInViewport() const
{
	return globalInputDevice->MousePosition;
}
