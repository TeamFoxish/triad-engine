#include "ScriptLoader.h"
#include "ScriptSystem.h"
#include "logs/Logs.h"
#include <string>
#include <iostream>
#include <filesystem>
#include "file/FileIO.h"

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
        CScriptBuilder* builder;
        if (_loadedModules.contains(moduleName)) {
            builder = _loadedModules[moduleName];
        } else {
            builder = new CScriptBuilder();
            builder->StartNewModule(gScriptSys->GetRawEngine(), moduleName.c_str());
            _loadedModules[moduleName] = builder;
        }
        builder->AddSectionFromFile(sourcePath.c_str());
        LOG_INFO("Resource \"Script\" module \"{}\" from source \"{}\" was loaded", moduleName, sourcePath);
    }
}

void ScriptLoader::LoadAll(const std::string &scriptDirPath)
{
    Triad::FileIO::FPath assetsBasePath = "./assets";
    Triad::FileIO::IterateDirectory(Triad::FileIO::FPath(scriptDirPath), 
        [assetsBasePath] (YAML::Node scriptDesc, const Triad::FileIO::FPath path) {
            std::string resFilePath = std::filesystem::relative(path, assetsBasePath).generic_string();
            ResTag tag = ResTag(ToStrid("res://" + resFilePath));
            std::unique_ptr<ResourceLoader> loader = Factory_<ResourceLoader>::Create("script");
            loader->Load(tag, scriptDesc["content"]);
    }, true, ".script");
}

void ScriptLoader::Build() {
    for (auto& [module, builder] : _loadedModules) {
        builder->BuildModule();
        LOG_INFO("Script module \"{}\" successfully built.", module);
    }
}

CScriptBuilder* ScriptLoader::GetBuilderByModule(const std::string &module)
{
    return _loadedModules[module];
}

std::vector<std::string> ScriptLoader::GetPropertyMetadata(int holderTypeId, asUINT fieldIdx)
{
    for (const auto& [moduleName, builder] : _loadedModules) {
        const std::vector<std::string> attributes = 
            builder->GetMetadataForTypeProperty(holderTypeId, (int)fieldIdx);
        if (!attributes.empty()) {
            return attributes;
        }
    }
    return std::vector<std::string>();
}
