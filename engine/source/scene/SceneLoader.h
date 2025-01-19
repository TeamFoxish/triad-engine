#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"
#include "scripts/ScriptObject.h"
#include "SceneTree.h"
#include <yaml-cpp/yaml.h>

#include <memory>
#include <unordered_map>
#include <queue>
#include <utility>

class Scene;

struct LinkageRequest {
	ScriptObject object;
	const std::string fieldName;
	const std::string ref;
	const uint64_t index;
	const bool isArrayField;
};

class SceneLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, SceneLoader> {
public:
	SceneLoader();
	~SceneLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override {}

	static std::unique_ptr<ScriptObject> CreateScene(ResTag tag);

	static void SaveScene(ResTag tag, SceneTree::Handle sceneRootHandle);

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<SceneLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "scene";
	}

	static void AddFieldToPendingState(ScriptObject* component, const std::string& fieldName, const std::string& ref) {
		_unlinkedComponentFiedls.push({*component, fieldName, ref, 0, false});
		const void* fieldPtr = component->GetField(fieldName);
		cachedComponentRefs[fieldPtr] = ref;
	}

	static void AddArrayFieldToPendingState(ScriptObject* component, const std::string& fieldName, uint64_t index, const std::string& ref) {
		_unlinkedComponentFiedls.push({*component, fieldName, ref, index, true});
		const void* fieldPtr = component->GetField(fieldName);
		std::vector<std::string>& refs = cachedArrayComponentRefs[fieldPtr];
		if (refs.size() == index) {
			refs.push_back(ref);
			return;
		} 
		if (refs.size() < index) {
			refs.resize(index + 1);
		}
		refs[index] = ref;
	}

	static const std::string& GetCachedComponentRef(const void* fieldPtr);
	static const std::string& GetCachedArrayComponentRef(const void* fieldPtr, uint64_t idx);
	static const std::vector<std::string>& GetCachedArrayComponentRefs(const void* fieldPtr);

	static ResTag GetSceneTag(const ScriptObject& obj);

	static YAML::Node BuildSceneYaml(SceneTree::Handle sceneRootHandle);

	static void RemoveEmptyOverrides(YAML::Node& scnNode);

	// only for components that are part of scene file
	static void AddSpawnedComponent(const ScriptObject& obj, const YAML::Node& desc);
	static void RemoveSpawnedComponent(SceneTree::Handle handle);
	static std::optional<YAML::Node> FindSpawnedComponent(const ScriptObject& obj);
	static void UpdateSpawnedComponentTransform(SceneTree::Handle handle);

private:
	// Links components refs to actual components
	static void LinkPass(const std::unique_ptr<ScriptObject>& root);

private:
	static inline std::unordered_map<ResTag, const YAML::Node> _scenes;
	// Stores component's fields that should be linked during linking pass
	static inline std::queue<LinkageRequest> _unlinkedComponentFiedls;

	static inline std::unordered_map<asIScriptObject*, ResTag> cachedSceneTags;

	// TODO: use Strid for cached refs to save memory on identical refs?
	static inline std::unordered_map<const void*, std::string> cachedComponentRefs;
	static inline std::unordered_map<const void*, std::vector<std::string>> cachedArrayComponentRefs;

	// idea: cache all prefabs and components spawned with overrides
	//       when any property gets changed by inspector, iterate upward from entity to find its root holder in cache and change property there??
	static inline std::unordered_map<asIScriptObject*, YAML::Node> spawnedComponents;
};
