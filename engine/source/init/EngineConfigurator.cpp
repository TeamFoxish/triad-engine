#include "EngineConfigurator.h"

#include "EngineCmdParams.h"
#include "EngineCmdParser.h"

#include "runtime/EngineRuntime.h"
#include "config/ConfigSystem.h"

bool EngineConfigurator::CreateRuntime(int argc, char* argv[], std::unique_ptr<EngineRuntime>& runtime)
{
    const auto params = std::make_shared<EngineCmdParams>();
    if (!EngineCmdParser::Parse(argc, argv, params)) {
        return false;
    }
    return CreateRuntime(*params, runtime);
}

bool EngineConfigurator::CreateRuntime(const EngineCmdParams& params, std::unique_ptr<EngineRuntime>& runtime)
{
    ConfigSystem* cfgSys = ConfigSystem::Instance();
    // engine config
    if (!cfgSys->Init(EngineCmdParams::DEFAULT_CONFIG_FILEPATH) || 
        (params.engineCfg && !cfgSys->Init(*params.engineCfg))) {
        return false;
    }
    // project description + engine overrides
    if (!cfgSys->Override(EngineCmdParams::DEFAULT_PROJECT_FILEPATH) ||
        (params.projectCfg && !cfgSys->Override(*params.projectCfg))) {
        return false;
    }

    runtime = std::make_unique<EngineRuntime>();

    if (params.startupScene) {
        // TODO: set scene to runtime?
    }

    return true;
}
