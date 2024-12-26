#include "SceneLoader.h"

#include "Scene.h"
#include "angelscript.h"
#include "game/PrefabLoader.h"
#include "scripts/ScriptSystem.h"
#include "scriptarray.h"
#include "logs/Logs.h"
#include <memory>

SceneLoader::SceneLoader()
{
	FACTORY_INIT;
}

SceneLoader::~SceneLoader()
{
}

void SceneLoader::Load(ResTag tag, const YAML::Node& desc)
{
	_scenes.emplace(tag, desc);
	LOG_INFO("Resource \"Scene\" with tag \"{}\" was indexed.", tag.string());
}

std::unique_ptr<ScriptObject> SceneLoader::CreateScene(ResTag tag)
{
	const YAML::Node sceneDesc = _scenes[tag];
	std::unique_ptr<ScriptObject> root = std::unique_ptr<ScriptObject>(new ScriptObject {"Engine", "Scene", {{asTYPEID_MASK_OBJECT, nullptr}}}); // currently scene has no parent

	for (const auto& componentDesc : sceneDesc["objects"]) {
		const std::string name = componentDesc.first.Scalar();
		const YAML::Node parameters = componentDesc.second;
		YAML::Node prefabNode = parameters["prefab"];
		YAML::Node componentNode = parameters["component"];
		ResTag objectTag;
		if (prefabNode.IsDefined()) {
			objectTag = ResTag(ToStrid(prefabNode.Scalar()));
		}
		if (componentNode.IsDefined()) {
			objectTag = ResTag(ToStrid(componentNode.Scalar()));
		}
		ScriptObject* component = PrefabLoader::Create(&objectTag, root.get());
		if (!component) {
			LOG_WARN("Failed to load component or prefab \"{}\"", objectTag.string());
			continue;
		}
		component->ApplyOverrides(parameters["overrides"]);
		component->SetField("name", name);
	}
	root->SetField("name", new std::string(sceneDesc["name"].Scalar()));
	LinkPass(root);
	return root;
}

std::unique_ptr<ScriptObject> GetChild(const std::unique_ptr<ScriptObject>& root, const std::string& childName) {
	CScriptArray* children = static_cast<CScriptArray*>(root->GetField("children"));
	for (asUINT i = 0; i < children->GetSize(); i++) {
		asIScriptObject* childRaw = *static_cast<asIScriptObject**>(children->At(i));
		auto child = std::unique_ptr<ScriptObject>(new ScriptObject {childRaw} );
		std::string* name = static_cast<std::string*>(child->GetField("name"));
		if (*name == childName) {
			return child;
		}
	}
	return nullptr;
}

std::unique_ptr<ScriptObject> GetComponentByAbsolutePath(const std::unique_ptr<ScriptObject>& root, const std::string& path) {
	std::string delimiter = "/";
	auto endPos = path.find(delimiter);
	if (endPos != std::string::npos) {
		std::string nextElemName = path.substr(0, path.find(delimiter));
		std::unique_ptr<ScriptObject> child = GetChild(root, nextElemName);
		if (child == nullptr) {
			return nullptr;
		}
		return GetComponentByAbsolutePath(child, path.substr(endPos + 1, path.length()));
	} else {
		std::unique_ptr<ScriptObject> child = GetChild(root, path);
		if (child == nullptr) {
			return nullptr;
		} else {
			return child;
		}
	}
	return nullptr;
}

void SceneLoader::LinkPass(const std::unique_ptr<ScriptObject>& root)
{
	while (!_unlinkedComponentFiedls.empty()) {
		LinkageRequest linkageRequest = _unlinkedComponentFiedls.front();
		if (linkageRequest.isArrayField) {
			// not implemented
		} else {
			std::unique_ptr<ScriptObject> linkedComponent = GetComponentByAbsolutePath(root, linkageRequest.ref);
			linkageRequest.object.SetField(linkageRequest.fieldName, linkedComponent->GetRaw());
		}
		LOG_INFO("Linked field \"{}\" to component \"{}\"", linkageRequest.fieldName, linkageRequest.ref);
		_unlinkedComponentFiedls.pop();
	}
	LOG_INFO("Scene linkage pass done");
}
