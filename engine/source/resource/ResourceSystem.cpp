#include "ResourceSystem.h"

#include "config/ConfigVar.h"
#include "ResourceLoader.h"
#include "misc/Factory.h"
#include <iostream>
#include <set>
#include <yaml-cpp/yaml.h>

static ConfigVar<std::vector<std::string_view>> cfgResRoots("/Project/Resource/Roots", {});

std::unique_ptr<ResourceSystem> gResourceSys = nullptr;

using namespace Triad::FileIO;

void ResourceSystem::LoadResource(ResTag tag)
{
    assert(tag != 0);
    const auto iter = resources.find(tag);
    if (iter != resources.end()) {
        iter->second.loadRefs++;
        return; // resource is loaded
    }
    LoadResourceImpl(tag);
}

void ResourceSystem::LoadResourceImpl(ResTag tag)
{
    ResPath filePath;
    if (!ResolveTagToFile(tag, filePath)) {
        std::cerr << std::format("unable to load resource with tag '{}'", tag.string()) << std::endl;
        return;
    }
    std::string data;
    try {
        data = ReadFile(filePath);
    } catch (FException ex) {
        std::cout << std::format("unable to read file '{}': {}", filePath.string(), ex.what()) << std::endl;
        return;
    }
    YAML::Node resFile;
    std::string type;
    try {
        resFile = YAML::Load(data);
        type = resFile["type"].as<std::string>();
    } catch (YAML::Exception ex) {
        std::cout << std::format("unable to read file '{}': {}", filePath.string(), ex.what()) << std::endl;
        return;
    }
    const YAML::Node desc = resFile["content"];
    Resource& res = resources[tag];
    res.tag = tag;
    res.type = ToStrid(type);
    res.loadRefs = 1;

    // fill up referenced resources
    std::set<ResTag> refs;
    std::vector<YAML::Node> pending;
    for (const auto& node : desc) {
        pending.push_back(node.second);
    }
    while (!pending.empty()) {
        const YAML::Node node = std::move(pending.back());
        pending.pop_back();
        for (const auto& child : node) {
            pending.push_back(child.second);
        }
        if (!node.IsScalar()) {
            continue;
        }
        const std::string& val = node.Scalar();
        if (IsTag(val)) {
            ResTag refTag = ToStrid(val);
            if (refTag == tag) {
                continue; // ignore self references
            }
            refs.insert(refTag);
            continue;
        }
    }
    res.refs.insert(res.refs.end(), refs.begin(), refs.end());
    for (ResTag ref : res.refs) {
        LoadResource(ref);
    }

    // create loader by type string and call Load
    if (!Factory<ResourceLoader>::IsRegistered(type.c_str())) {
        std::cout << std::format("unable to load resource with tag '{}'. there is no registered loader for type '{}'", tag.string(), type) << std::endl;
        return;
    }
    std::unique_ptr<ResourceLoader> loader = Factory<ResourceLoader>::Create(type.c_str());
    loader->Load(tag, desc);
}

void ResourceSystem::UnloadResource(ResTag tag)
{
    assert(tag != 0);
    const auto iter = resources.find(tag);
    if (iter == resources.end()) {
        std::cout << std::format("unable to unload resource with tag '{}'. it wasn't loaded", tag.string()) << std::endl;
        return;
    }
    Resource& res = iter->second;
    if (--res.loadRefs > 0) {
        return; // resource is still required
    }
    // unload all referenced resources
    for (ResTag ref : res.refs) {
        UnloadResource(ref);
    }
    // create loader by type string and call Unload
    if (!Factory<ResourceLoader>::IsRegistered(res.type.string())) {
        std::cout << std::format("unable to unload resource with tag '{}'. there is no registered loader for type '{}'", tag.string(), res.type.string()) << std::endl;
        return;
    }
    std::unique_ptr<ResourceLoader> loader = Factory<ResourceLoader>::Create(res.type.string());
    loader->Unload(tag);
}

bool ResourceSystem::ResolveTagToFile(ResTag tag, ResPath& outPath) const
{
    for (std::string_view root : cfgResRoots.GetRef()) {
        if (ResolveTagToFile(tag, root, outPath)) {
            return true;
        }
    }
    return false;
}

bool ResourceSystem::ResolveTagToFile(ResTag tag, const ResPath& root, ResPath& outPath) const
{
    const ResPath subpath = GetTagSubpath(tag);
    outPath = root / subpath;
    return std::filesystem::exists(outPath);
}

ResPath ResourceSystem::GetTagSubpath(ResTag tag)
{
    const std::string_view tagStr = tag.string();
    assert(tagStr.size() > TAG_PREFIX.size());
    return tagStr.substr(TAG_PREFIX.size());
}

bool ResourceSystem::IsTag(std::string_view str)
{
    return str.starts_with(TAG_PREFIX);
}
