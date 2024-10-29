#pragma once

#include "RuntimeIface.h"

#include <string>
#include "config/ConfigVar.h"

class Window;

class EngineRuntime : public RuntimeIface {
public:
	bool Init(const InitParams& params) override;
	void Run() override;
	void Shutdown() override;

	Window* GetWindow() const override { return window; }

private:
	Window* window = nullptr;
};

extern ConfigVar<std::string_view> cfgProjectName;
