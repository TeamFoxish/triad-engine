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
	CScriptArray* child = CScriptArray::Create(childArrayType, 16);

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
	return root;
}