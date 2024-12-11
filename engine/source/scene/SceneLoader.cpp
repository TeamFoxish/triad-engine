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
	asIScriptEngine* engine = gScriptSys->GetRawEngine();
	ScriptObject* root = new ScriptObject("Engine", "Scene");
	asITypeInfo* childArrayType = engine->GetTypeInfoByDecl("array<Component@>");
	CScriptArray* child = CScriptArray::Create(childArrayType);

	for (const auto& componentDesc : sceneDesc["objects"]) {
		const std::string name = componentDesc.first.Scalar();
		const YAML::Node parameters = componentDesc.second;
		ResTag prefabTag = ResTag(ToStrid(parameters["prefab"].Scalar()));
		ScriptObject* component = PrefabLoader::Create(&prefabTag);
		component->ApplyOverrides(parameters["overrides"]);
		asIScriptObject* valObj = component->GetRaw();
		child->InsertLast(&valObj);
	}
	root->SetField("child", child);
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
	_componentRegistry.clear();
	LOG_INFO("Scene linkage pass done");
}
