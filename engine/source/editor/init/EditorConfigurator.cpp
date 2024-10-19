#ifdef EDITOR

#include "EditorConfigurator.h"

#include "EditorCmdParams.h"
#include "init/EngineCmdParser.h"
#include "init/EngineConfigurator.h"
#include "editor/runtime/EditorRuntime.h"
#include "runtime/EngineRuntime.h"
#include "config/ConfigSystem.h"

bool EditorConfigurator::CreateRuntime(int argc, char* argv[], std::unique_ptr<EditorRuntime>& runtime)
{
    const auto params = std::make_shared<EditorCmdParams>();
    if (!EngineCmdParser::Parse(argc, argv, params)) {
        return false;
    }
    std::unique_ptr<EngineRuntime> engineRuntime;
    EngineConfigurator{}.CreateRuntime(*params, engineRuntime);
    // project description + engine overrides
    ConfigSystem* cfgSys = ConfigSystem::Instance();
    if (!cfgSys->Override(EditorCmdParams::DEFAULT_CONFIG_FILEPATH) ||
        (params->editorCfg && !cfgSys->Override(*params->editorCfg))) {
        return false;
    }
    runtime = std::make_unique<EditorRuntime>(std::move(engineRuntime));
    return true;
}

#endif // EDITOR
