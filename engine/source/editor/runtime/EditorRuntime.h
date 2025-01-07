#pragma once

#ifdef EDITOR

#include "runtime/RuntimeIface.h"

#include "EditorController.h"

#include <memory>

class EngineRuntime;

class EditorRuntime : public RuntimeIface {
public:
	EditorRuntime(std::unique_ptr<EngineRuntime> engineRuntime);
	EditorRuntime(const EditorRuntime&) = delete;
	EditorRuntime(EditorRuntime&&) = delete;
	~EditorRuntime();

	bool Init(const InitParams& params) override;
	void Run() override;
	void Shutdown() override;

	// TODO: replace with viewport?
	Window* GetWindow() const override;

	Math::Vector2 GetMousePosInViewport() const override;

private:
	void HandleSimulationStart();
	void HandleSimulationEnd();

private:
	EditorController controller;

	std::unique_ptr<EngineRuntime> engRuntime;
};

#endif EDITOR
