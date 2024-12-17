#pragma once

#include <string>
#include <vector>

#ifdef EDITOR
#include "UIOutliner.h"
#endif // EDITOR

class Window;
class Component;
class CompositeComponent;
class Outliner;


class UIDebug
{
public:
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
private:
	static void DrawGizmo();
	
public:
	static inline Outliner outliner; // TODO: remove
#endif // EDITOR
};