#pragma once

#include "RuntimeIface.h"

#include <memory>
#include <string>
#include "config/ConfigVar.h"
#include "misc/Delegates.h"

class Window;

class EngineRuntime : public RuntimeIface {
public:
	EngineRuntime() = default;
	EngineRuntime(const EngineRuntime&) = delete;
	EngineRuntime(EngineRuntime&&) = delete;
	~EngineRuntime() = default;

	bool Init(const InitParams& params) override;
	void Run() override;
	void Shutdown() override;

	Window* GetWindow() const override { return window; }

private:
	Window* window = nullptr;
};

extern ConfigVar<std::string_view> cfgProjectName;

extern std::unique_ptr<class Game> gTempGame;

extern MulticastDelegate<int, int> gViewportResized;
