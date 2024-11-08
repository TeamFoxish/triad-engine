#pragma once

class Window;

class UIDebug
{
public:
	static void Init(Window* window);

	static void StartNewFrame();

	static void TestDraw();

	static void Render();

	static void Destroy();

	static bool GetIsInitFlag();

protected:
	static inline bool isInitted = false;
};