#include "AssetManager.h"

#include "config/ConfigVar.h"
#include "logs/Logs.h"

#include <inja/inja.hpp>

using namespace Triad;

static ConfigVar<std::string_view> cfgTemplatesPath("/Editor/AssetManager/TemplatesPath", "templates");
static ConfigVar<std::string_view> cfgScriptsDir("/Editor/AssetManager/ScriptsDir", "assets/scripts");

template<AssetManager::AssetType Type>
struct AssetFactory;

template<>
struct AssetFactory<AssetManager::AssetType::Script> {
    static nlohmann::json Data(AssetManager* manager, const AssetManager::CreateInfo& info)
    {
        nlohmann::json data;
        Triad::FileIO::FPath scriptPath = cfgScriptsDir.GetRef() / info.name;
        scriptPath.replace_extension("as");
        data["script"]["path"] = scriptPath.string();
        data["script"]["module"] = "Engine";
        return data;
    }

    static void Create(AssetManager* manager, const AssetManager::CreateInfo& info, bool isComponent, bool isComposite)
    {
        nlohmann::json data = Data(manager, info);
        AssetManager::CreateInfo scriptInfo = info;
        scriptInfo.dir = cfgScriptsDir.GetRef();
        manager->CreateAsset(scriptInfo, "script", data);
        if (isComponent) {
            data["component"]["isComposite"] = isComposite;
            data["component"]["name"] = info.name.string();
            manager->CreateAsset(scriptInfo, "component.as", data);
        } else {
            data["script"]["name"] = info.name.string();
            manager->CreateAsset(scriptInfo, "as", data);
        }
    }
};

template<>
struct AssetFactory<AssetManager::AssetType::Component> {
    static void Create(AssetManager* manager, const AssetManager::CreateInfo& info)
    {
        nlohmann::json data;
        data["component"]["name"] = info.name.string();
        data["script"]["module"] = "Engine";
        manager->CreateAsset(info, "component", data);
        AssetFactory<AssetManager::AssetType::Script>::Create(manager, info, true, info.isComposite);
    }
};

void AssetManager::Init()
{
    FileIO::FPath templatesPath(cfgTemplatesPath.GetRef());
    if (!FileIO::exists(templatesPath)) {
        LOG_ERROR("failed to initialize editor's AssetManager. unable to locate templates directory: '{}'", cfgTemplatesPath.GetRef());
        return;
    }
    for (const FileIO::directory_entry& entry : FileIO::recursive_directory_iterator(templatesPath)) {
        const auto temp = entry.path().extension();
        if (entry.is_directory() || entry.path().extension() != TEMPLATE_EXT) {
            continue;
        }
        const Strid type = ToStrid(entry.path().filename().replace_extension().string());
        registeredTemplates[type] = entry.path();
    }
}

void AssetManager::CreateAsset(const CreateInfo& info)
{
    switch(info.type) {
        case AssetType::Component:
            AssetFactory<AssetType::Component>::Create(this, info);
            break;
        case AssetType::Script:
            AssetFactory<AssetType::Script>::Create(this, info, false, info.isComposite);
            break;
    }
}

void AssetManager::CreateAsset(const CreateInfo& info, std::string_view ext, const nlohmann::json& data)
{
    Triad::FileIO::FPath path = info.dir / info.name;
    Triad::FileIO::FPath tempExt = ext;
    path.replace_extension(tempExt.has_extension() ? tempExt.extension() : tempExt);

    const auto iter = registeredTemplates.find(ToStrid(ext));
    if (iter == registeredTemplates.end()) {
        LOG_ERROR("failed to create asset '{}'. no template was registerd for type '{}'", path.string(), ext);
        return;
    }
    
    const Triad::FileIO::FPath& templatePath = iter->second;
    std::string content;
    try {
        content = FileIO::ReadFile(templatePath);
    } catch (FileIO::FException ex) {
        LOG_ERROR("Unable to read file \"{}\": {}", templatePath.string(), ex.what());
        return;
    }

    inja::Environment env;
    try {
        const inja::Template temp = env.parse(content);
        env.write(temp, data, path.string());
    }
    catch (inja::InjaError ex) {
        LOG_ERROR("failed to create asset '{}' from template '{}': {}", path.string(), templatePath.string(), ex.what());
        return;
    }
}
