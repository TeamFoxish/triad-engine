#pragma once

#include "resource/Res.h"

#include <nlohmann/json.hpp>
#include <unordered_map>

class AssetManager {
public:
    enum class AssetType {
        None,
        Component,
        Script,
        Material
    };

    AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;
    ~AssetManager() = default;

    void Init();

    struct CreateInfo {
        AssetType type;
        ResPath dir;
        ResPath name;
        bool isComposite = false; // TEMP
    };
    void CreateAsset(const CreateInfo& info);
    void CreateAsset(const CreateInfo& info, std::string_view ext, const nlohmann::json& data);

private:
    std::unordered_map<Strid, Triad::FileIO::FPath> registeredTemplates;

    static constexpr std::string_view TEMPLATE_EXT = ".inja";
};
