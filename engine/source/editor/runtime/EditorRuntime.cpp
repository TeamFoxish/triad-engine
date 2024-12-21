#ifdef EDITOR

#include "EditorRuntime.h"

#include "runtime/EngineRuntime.h"
#include "os/Window.h"

#include "editor/ui_debug/UIDebug.h"
#include "math/Math.h"

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
	return engRuntime->Init(engParams);
}

void EditorRuntime::Run()
{
	engRuntime->Run();
}

void EditorRuntime::Shutdown()
{
	engRuntime->Shutdown();
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
#endif // EDITOR
