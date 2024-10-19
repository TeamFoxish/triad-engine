#ifdef EDITOR

#include "EditorRuntime.h"

#include "runtime/EngineRuntime.h"

EditorRuntime::EditorRuntime(std::unique_ptr<EngineRuntime> engineRuntime)
	: engRuntime(std::move(engineRuntime))
{

}

EditorRuntime::~EditorRuntime()
{
}

#endif // EDITOR
