#pragma once

#include <string>
#include <vector>
#include <memory>

#ifdef EDITOR
#include "UIOutliner.h"
#include "UIContentBrowser.h"
#include "editor/runtime/EditorController.h"

#include "misc/Delegates.h"
#endif // EDITOR

class Window;
class Component;
class CompositeComponent;


class UIDebug
{
public:
#ifdef EDITOR
	class ViewportInputContext : public EditorController::InputContextBase {
	public:
		void ProceedInput(InputDevice* device) override;

		InputTarget GetInputTarget() const override { return InputTarget::Game; }
	};
#endif

	static void Init(Window* window);

	static void StartNewFrame();

	// ToDo: rename
	static void TestDraw();

	static void Render();

	static void Destroy();

	static bool GetUIDebugFlag();

	static int GetViewportX() { return viewportX; }
	static int GetViewportY() { return viewportY; }

	static inline bool useDefaultSettings = true;
	static inline bool start_simulation = false;
	static inline bool gizmoSelected = false;

protected:
	static void UpdateViewportPos();
	static bool HandleViewportResize();

protected:
	static inline bool isInitted = false;
	static inline bool isSceneFocused = false;

	static inline int viewportX = 0;
	static inline int viewportY = 0;
	static inline int viewportWidth = -1;
	static inline int viewportHeight = -1;

#ifdef EDITOR
public:
	static Outliner& GetOutliner() { return outliner; }

private:
	static void DrawGizmo();
	static void DrawAdditionalFields(ScriptObject* obj);
	
public:
	static inline Outliner outliner; // TODO: remove
	static inline ContentBrowser contentBrowser; // TODO: remove

	static inline std::shared_ptr<ViewportInputContext> viewportInpContext;

	static inline MulticastDelegate<> onSimulationStart;
	static inline MulticastDelegate<> onSimulationEnd;
#endif // EDITOR
};