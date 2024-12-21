#pragma once

#include "math/Math.h"

class RuntimeIface {
public:
	struct InitParams {
		struct Window {
			bool createWindow = true;
			int width = 800;
			int height = 800;
		} window;
	};

public:
	virtual bool Init(const InitParams& params) = 0;
	virtual void Run() = 0;
	virtual void Shutdown() = 0;

	virtual class Window* GetWindow() const = 0;

	virtual Math::Vector2 GetMousePosInViewport() const = 0;
};

inline RuntimeIface* gEngineRuntime = nullptr;
