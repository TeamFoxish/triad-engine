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

struct OutlinerNode
{
	std::string name = "empty node";
	bool isSelected = false;
	std::vector<OutlinerNode*> children;
};

class UIDebug
{
public:
	static void Init(Window* window);

	static void StartNewFrame();

	static void TestDraw();

	static void Render();

	static void Destroy();

	static bool GetUIDebugFlag();

	static inline bool useDefaultSettings = true;

protected:
	static bool HandleViewportResize();

protected:
	static inline bool isInitted = false;
	static inline bool isSceneFocused = false;

	static inline int viewportWidth = -1;
	static inline int viewportHeight = -1;

#ifdef EDITOR
	static inline Outliner outliner;
#endif // EDITOR
};