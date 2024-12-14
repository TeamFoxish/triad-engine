#include "SceneLoader.h"

#include "Scene.h"
#include "angelscript.h"
#include "game/PrefabLoader.h"
#include "scripts/ScriptSystem.h"
#include "scriptarray.h"
#include "logs/Logs.h"

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

ScriptObject* SceneLoader::CreateScene(ResTag tag)
{
	const YAML::Node sceneDesc = _scenes[tag];
	ScriptObject* root = new ScriptObject("Engine", "Scene", {{asTYPEID_MASK_OBJECT, nullptr}}); // currently scene has no parent

	for (const auto& componentDesc : sceneDesc["objects"]) {
		const std::string name = componentDesc.first.Scalar();
		const YAML::Node parameters = componentDesc.second;
		ResTag prefabTag = ResTag(ToStrid(parameters["prefab"].Scalar()));
		ScriptObject* component = PrefabLoader::Create(&prefabTag, root);
		if (!component) {
			continue; // TEMP?
		}
		component->ApplyOverrides(parameters["overrides"]);
	}
	root->SetField("name", new std::string(sceneDesc["name"].Scalar()));
	LinkPass();
	return root;
}

void SceneLoader::LinkPass()
{
	while (!_unlinkedComponentFiedls.empty()) {
		LinkageRequest linkageRequest = _unlinkedComponentFiedls.front();
		if (linkageRequest.isArrayField) {
			// not implemented
		} else {
			ScriptObject* linkedComponent = _componentRegistry[linkageRequest.ref];
			linkageRequest.object->SetField(linkageRequest.fieldName, linkedComponent->GetRaw());
		}
		LOG_INFO("Linked field \"{}\" to component \"{}\"", linkageRequest.fieldName, linkageRequest.ref);
		_unlinkedComponentFiedls.pop();
	}
	for( const auto& [id, component] : _componentRegistry) {
		delete component;
	}
	_componentRegistry.clear();
	LOG_INFO("Scene linkage pass done");
}
