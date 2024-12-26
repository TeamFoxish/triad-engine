#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"
#include "scripts/ScriptObject.h"
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

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<SceneLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "scene";
	}

	static void AddFieldToPendingState(ScriptObject* component, const std::string& fieldName, const std::string ref) {
		_unlinkedComponentFiedls.push({*component, fieldName, ref, 0, false});
	}

	static void AddArrayFieldToPendingState(ScriptObject* component, const std::string& fieldName, uint64_t index, const std::string ref) {
		_unlinkedComponentFiedls.push({*component, fieldName, ref, index, true});
	}

private:
	static inline std::unordered_map<ResTag, const YAML::Node> _scenes;
	// Stores component's fields that should be linked during linking pass
	static inline std::queue<LinkageRequest> _unlinkedComponentFiedls;

	// Links components refs to actual components
	static void LinkPass(const std::unique_ptr<ScriptObject>& root);
};
