#pragma once

#include "angelscript.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "scripts/ScriptObject.h"
#include "resource/ResourceLoader.h"
#include "scene/SceneTree.h"
#include "misc/Factory.h"

class ComponentLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, ComponentLoader> {
public:
    ComponentLoader();
	~ComponentLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override {};

	static const YAML::Node& GetComponentDesc(ResTag tag);

    static ScriptObject* CreateComponent(ResTag tag, ScriptObject* parent = nullptr, const YAML::Node* sceneRepr = nullptr);

    static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<ComponentLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "component";
	}

	static void SetComponentName(ScriptObject& obj, const std::string& name);

	static ResTag GetComponentTag(const ScriptObject& obj);

	static void AddComponentTag(const ScriptObject& obj, ResTag tag);

	static YAML::Node BuildCompYaml(SceneTree::Handle entHandle, const YAML::Node& origDesc);

	static YAML::Node BuildOverridesList(SceneTree::Handle entHandle, const YAML::Node& origDesc);

	static void PopulateEmptySceneYaml(YAML::Node& prefabNode, ResTag tag);

private:
	static inline std::unordered_map<ResTag, const YAML::Node> _components;

	static inline std::unordered_map<ResTag, const YAML::Node> defaultStates;

	static inline std::unordered_map<asIScriptObject*, ResTag> cachedCompTags;
};