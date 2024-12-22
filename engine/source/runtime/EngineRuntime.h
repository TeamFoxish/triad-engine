#pragma once

#include "RuntimeIface.h"

#include <memory>
#include <string>
#include "config/ConfigVar.h"
#include "misc/Delegates.h"

class Window;

class EngineRuntime : public RuntimeIface {
public:
	struct FrameParams {
		bool simulationEnabled = true;
	};

	EngineRuntime() = default;
	EngineRuntime(const EngineRuntime&) = delete;
	EngineRuntime(EngineRuntime&&) = delete;
	~EngineRuntime() = default;

	bool IsRunning() const { return isRunning; }

	bool Init(const InitParams& params) override;
	void Run() override;
	void RunSingleFrame(FrameParams&& params = {});
	void Shutdown() override;

	Window* GetWindow() const override { return window; }

	Math::Vector2 GetMousePosInViewport() const override;

private:
	void UpdateSoundListener();

private:
	Window* window = nullptr;

	bool isRunning = true;
};

extern ConfigVar<std::string_view> cfgProjectName;

extern std::unique_ptr<class Game> gTempGame;

extern MulticastDelegate<int, int> gViewportResized;
