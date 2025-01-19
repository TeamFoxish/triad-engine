#include "PrefabLoader.h"

#include "math/Math.h"

#include "render/RenderSystem.h"
#include "render/RenderResources.h"
#include "render/Renderer.h"
#include "render/material/Material.h"

#include "game/Game.h"
#include "components/PlayerBall.h"
#include "scripts/ScriptObject.h"
#include "scripts/ScriptSystem.h"
#include "scripts/ComponentLoader.h"
#include "scene/SceneLoader.h"
#include "logs/Logs.h"

#include "runtime/EngineRuntime.h"

PrefabLoader::PrefabLoader()
{
	FACTORY_INIT;
}

PrefabLoader::~PrefabLoader()
{
}

void PrefabLoader::Load(ResTag tag, const YAML::Node& desc)
{
	_prefabs.emplace(tag, desc);
	LOG_INFO("Resource \"Prefab\" with tag \"{}\" was indexed", tag.string());
}

ScriptObject* PrefabLoader::Create(ResTag tag, ScriptObject* parent, YAML::Node* sceneRepr)
{
	const YAML::Node desc = _prefabs[tag];

	// TEMP
	if (desc["class"] && desc["class"].Scalar() == "player") {
		auto player = new PlayerBall(gTempGame.get());
		player->Initialize();
		return nullptr;
	}

	asIScriptEngine* engine = gScriptSys->GetRawEngine();
	ScriptObject::ArgsT args;
	if (parent) {
		args = {{asTYPEID_MASK_OBJECT, parent->GetRaw()}};
	}
	ScriptObject* root = new ScriptObject("Engine", "CompositeComponent", std::move(args)); // TODO: prefab may have a different scriptable type as root (not CompositeComponent)

	// scene node reference
	YAML::Node childrenSceneNode;
	if (sceneRepr) {
		const YAML::Node& childrenRef = (*sceneRepr)["overrides"]["children"];
		if (childrenRef) {
			childrenSceneNode = childrenRef;
		} else {
			childrenSceneNode = (*sceneRepr)["overrides"]["children"] = YAML::Node();
		}
	}
	for (const auto& componentDesc : desc["components"]) {
		const std::string& name = componentDesc.first.Scalar();
		const YAML::Node parameters = componentDesc.second;
		ResTag componentTag = ResTag(ToStrid(parameters["component"].Scalar()));
		const std::string nameInScene = !name.starts_with('$') ? '$' + name : name;
		YAML::Node childSceneNode = childrenSceneNode[nameInScene];
		if (!childSceneNode) {
			// child component wasn't met in overrides
			childSceneNode = childrenSceneNode[nameInScene] = YAML::Node();
		}
		ScriptObject* component = ComponentLoader::CreateComponent(componentTag, root, sceneRepr ? &childSceneNode : nullptr);
		component->ApplyOverrides(parameters["overrides"]);
		ComponentLoader::SetComponentName(*component, name);
		// TODO: delete component; check that nothing breaks (or better return ComponentLoader::Create by value)
	}

	cachedPrefabTags[root->GetRaw()] = tag;
	if (sceneRepr) {
		YAML::Node prefabNode = *sceneRepr;
		if (prefabNode.IsNull()) {
			PopulateEmptySceneYaml(prefabNode, tag);
		}
		SceneLoader::AddSpawnedComponent(*root, prefabNode);
	}

	return root;
}

YAML::Node PrefabLoader::BuildPrefabYaml(SceneTree::Handle instHandle, std::string& keyOut)
{
	YAML::Node root;
	const SceneTree::Entity& entity = gSceneTree->Get(instHandle);
	keyOut = entity.name;

	ResTag tag = GetPrefabTag(entity.obj);
	if (tag == ResTag{}) {
		LOG_ERROR("failed to build prefab yaml for '{}'. unable to get cached prefab tag", entity.name);
		return root;
	}
	root["prefab"] = tag.string();

	assert(entity.obj.GetRaw());
	assert(entity.isComposite); // revisit this assert if prefab will ever be non-composite
	const auto descIter = _prefabs.find(tag);
	if (descIter == _prefabs.end()) {
		// TODO: log error
		return root;
	}
	YAML::Node rootComp = ComponentLoader::BuildCompYaml(instHandle, descIter->second);
	//if (rootComp.size() == 0) {
	//	// log error?
	//	return root;
	//}
	//YAML::Node& childrenNode = rootComp["children"] = {};

	return root;
}

void PrefabLoader::PopulateEmptySceneYaml(YAML::Node& prefabNode, ResTag tag)
{
	prefabNode["prefab"] = tag.string();
}

ResTag PrefabLoader::GetPrefabTag(const ScriptObject& obj)
{
	assert(obj.GetRaw());
	const auto iter = cachedPrefabTags.find(obj.GetRaw());
	return iter != cachedPrefabTags.end() ? iter->second : ResTag{};
}
