#include "ScriptLoader.h"
#include "ScriptSystem.h"

ScriptLoader::ScriptLoader()
{
    FACTORY_INIT;
}

ScriptLoader::~ScriptLoader()
{
}

void ScriptLoader::Load(ResTag tag, const YAML::Node &desc)
{
    auto moduleName = desc["module"].Scalar();
    auto sourcePath = desc["source_file"].Scalar();
    if (!moduleName.empty() && !sourcePath.empty()) {
        CScriptBuilder builder;
        if (_loadedModules.contains(moduleName)) {
            builder = _loadedModules[moduleName];
        } else {
            builder.StartNewModule(gScriptSys->GetRawEngine(), moduleName.c_str());
            _loadedModules[moduleName] = builder;
        }
        builder.AddSectionFromFile(sourcePath.c_str());
    }
}

void ScriptLoader::Build() {
    for (auto& [module, builder] : _loadedModules) {
        builder.BuildModule();
    }
}
