#ifdef EDITOR

#include "EditorRuntime.h"

#include "runtime/EngineRuntime.h"
#include "os/Window.h"

#include "editor/ui_debug/UIDebug.h"
#include "math/Math.h"

#include "sound/SoundSystem.h"

static ConfigVar<int> cfgEditorWindowWidth("/Editor/Window/Width", 1280);
static ConfigVar<int> cfgEditorWindowHeight("/Editor/Window/Height", 720);

EditorRuntime::EditorRuntime(std::unique_ptr<EngineRuntime> engineRuntime)
	: engRuntime(std::move(engineRuntime))
{

}

EditorRuntime::~EditorRuntime()
{
}

bool EditorRuntime::Init(const InitParams& params)
{
	if (!gEngineRuntime) {
		gEngineRuntime = this;
	}
	InitParams engParams;
	engParams.window.width = cfgEditorWindowWidth;
	engParams.window.height = cfgEditorWindowHeight;

	UIDebug::onSimulationStart.AddRaw(this, &EditorRuntime::HandleSimulationStart);
	UIDebug::onSimulationEnd.AddRaw(this, &EditorRuntime::HandleSimulationEnd);

	assetManager.Init();

	return engRuntime->Init(engParams);
}

void EditorRuntime::Run()
{
	while (engRuntime->IsRunning()) {
		controller.ProceedInput();
		engRuntime->RunSingleFrame(EngineRuntime::FrameParams{
			.simulationEnabled = UIDebug::start_simulation
		});
	}
}

void EditorRuntime::Shutdown()
{
	engRuntime->Shutdown();
	UIDebug::onSimulationStart.RemoveObject(this);
}

Window* EditorRuntime::GetWindow() const
{
	return engRuntime->GetWindow();
}

Math::Vector2 EditorRuntime::GetMousePosInViewport() const
{
	Math::Vector2 pos = engRuntime->GetMousePosInViewport();
	pos.x -= UIDebug::GetViewportX();
	pos.y -= UIDebug::GetViewportY();
	return pos;
}

void EditorRuntime::HandleSimulationStart()
{
}

void EditorRuntime::HandleSimulationEnd()
{
	gSoundSys->StopAllSounds();
	gSoundSys->Update(0.0f);
}
#endif // EDITOR
