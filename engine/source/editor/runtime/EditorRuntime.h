#pragma once

#ifdef EDITOR

#include "runtime/RuntimeIface.h"

#include <memory>

class EngineRuntime;

class EditorRuntime : public RuntimeIface {
public:
	EditorRuntime(std::unique_ptr<EngineRuntime> engineRuntime);
	EditorRuntime(const EditorRuntime&) = delete;
	EditorRuntime(EditorRuntime&&) = delete;
	~EditorRuntime();

private:
	std::unique_ptr<EngineRuntime> engRuntime;
};

#endif EDITOR
