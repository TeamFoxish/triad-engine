#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"
#include "scripts/ScriptObject.h"
#include "scene/SceneTree.h"
#include <unordered_map>
#include <functional>
#include "logs/Logs.h"

class Component;

class PrefabLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, PrefabLoader> {
public:
	PrefabLoader();
	~PrefabLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override {}

	static const YAML::Node& GetPrefabDesc(ResTag tag);

	// TODO: replace sceneRepr pointer with std::optional?
	static ScriptObject* Create(ResTag tag, ScriptObject* parent = nullptr, YAML::Node* sceneRepr = nullptr);

	static YAML::Node BuildPrefabYaml(SceneTree::Handle instHandle, std::string& keyOut);

	static void PopulateEmptySceneYaml(YAML::Node& prefabNode, ResTag tag);

    static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<PrefabLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "prefab";
	}

	static ResTag GetPrefabTag(const ScriptObject& obj);

private:
	static inline std::unordered_map<ResTag, const YAML::Node> _prefabs;

	static inline std::unordered_map<asIScriptObject*, ResTag> cachedPrefabTags;
};
