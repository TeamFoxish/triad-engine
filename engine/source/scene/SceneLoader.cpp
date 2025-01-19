#include "SceneLoader.h"

#include "Scene.h"
#include "angelscript.h"
#include "game/PrefabLoader.h"
#include "scripts/ComponentLoader.h"
#include "scripts/ScriptSystem.h"
#include "scriptarray.h"
#include "resource/ResourceBuilder.h"
#include "shared/SharedStorage.h"
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
	const YAML::Node& sceneDesc = _scenes[tag];
	ScriptObject::ArgsT args = {{asTYPEID_MASK_OBJECT, nullptr}};
	std::unique_ptr<ScriptObject> root = std::make_unique<ScriptObject>("Engine", "CompositeComponent", std::move(args)); // currently scene has no parent

	YAML::Node editableSceneDesc = YAML::Clone(sceneDesc);
	AddSpawnedComponent(*root, editableSceneDesc);

	YAML::Node sceneObjectsDesc = editableSceneDesc["objects"];
	if (!sceneObjectsDesc) {
		sceneObjectsDesc = editableSceneDesc["objects"] = YAML::Node();
	}

	for (const auto& componentDesc : sceneDesc["objects"]) {
		const std::string& name = componentDesc.first.Scalar();
		const YAML::Node& parameters = componentDesc.second;
		const YAML::Node& prefabNode = parameters["prefab"];
		const YAML::Node& componentNode = parameters["component"];
		ResTag objectTag;
		if (prefabNode.IsDefined()) {
			objectTag = ResTag(ToStrid(prefabNode.Scalar()));
		}
		if (componentNode.IsDefined()) {
			objectTag = ResTag(ToStrid(componentNode.Scalar()));
		}
		YAML::Node childDesc = sceneObjectsDesc[name];
		if (!childDesc) {
			childDesc = sceneObjectsDesc[name] = YAML::Node();
		}
		ScriptObject* component = PrefabLoader::Create(objectTag, root.get(), &childDesc);
		if (!component) {
			LOG_WARN("Failed to load component or prefab \"{}\"", objectTag.string());
			continue;
		}
		component->ApplyOverrides(parameters["overrides"]);
		ComponentLoader::SetComponentName(*component, name);
		// TODO: delete component; check that nothing breaks (or better return PrefabLoader::Create by value) [if not planning ScriptObjectNative]
	}

	ComponentLoader::SetComponentName(*root, sceneDesc["name"].Scalar());
	LinkPass(root);
	cachedSceneTags[root->GetRaw()] = tag;

	return root;
}

void SceneLoader::SaveScene(ResTag tag, SceneTree::Handle sceneRootHandle)
{
	// build yaml representation of sceneObj
	// locate source file by tag
	// save scene

	
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

const std::string& SceneLoader::GetCachedComponentRef(const void* fieldPtr)
{
	static const std::string emptyStr;
	const auto iter = cachedComponentRefs.find(fieldPtr);
	return iter != cachedComponentRefs.end() ? iter->second : emptyStr;
}

const std::string& SceneLoader::GetCachedArrayComponentRef(const void* fieldPtr, uint64_t idx)
{
	static const std::string emptyStr;
	const auto iter = cachedArrayComponentRefs.find(fieldPtr);
	if (iter == cachedArrayComponentRefs.end() || idx >= iter->second.size()) {
		return emptyStr;
	}
	return iter->second[idx];
}

const std::vector<std::string>& SceneLoader::GetCachedArrayComponentRefs(const void* fieldPtr)
{
	static const std::vector<std::string> emptyArr;
	const auto iter = cachedArrayComponentRefs.find(fieldPtr);
	return iter != cachedArrayComponentRefs.end() ? iter->second : emptyArr;
}

ResTag SceneLoader::GetSceneTag(const ScriptObject& obj)
{
	assert(obj.GetRaw());
	const auto iter = cachedSceneTags.find(obj.GetRaw());
	return iter != cachedSceneTags.end() ? iter->second : ResTag{};
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

YAML::Node SceneLoader::BuildSceneYaml(SceneTree::Handle sceneRootHandle)
{
	YAML::Node root = ResourceBuilder::BuildEmpty("scene");
	const SceneTree::Entity& rootEntity = gSceneTree->Get(sceneRootHandle);
	assert(rootEntity.isComposite);

	std::optional<YAML::Node> editableSceneDesc = FindSpawnedComponent(rootEntity.obj);
	if (!editableSceneDesc) {
		LOG_ERROR("failed to build scene '{}' yaml repr. unable to find spawned scene editable desc", rootEntity.name);
		return root;
	}
	YAML::Node& desc = root["content"] = YAML::Clone(*editableSceneDesc);
	RemoveEmptyOverrides(desc);

	//desc["name"] = rootEntity.name;

	//YAML::Node& objects = desc["objects"] = {};
	//for (SceneTree::Handle childHandle : rootEntity.children) {
	//	// for each prefab take its desc from PrefabLoader and diff its overrides to current values
	//	// then for each component diff its overrides to current values
	//	std::string key;
	//	YAML::Node objNode = PrefabLoader::BuildPrefabYaml(childHandle, key);
	//	objects[key] = std::move(objNode);
	//}

	return root;
}

void SceneLoader::RemoveEmptyOverrides(YAML::Node& scnNode)
{
	std::vector<YAML::Node> pendingNodes;
	for (auto node : scnNode) {
		if (!node.second || !node.second.IsMap()) {
			continue;
		}
		pendingNodes.push_back(node.second);
	}
	while (!pendingNodes.empty()) {
		YAML::Node node = pendingNodes.back();
		pendingNodes.pop_back();
		YAML::Node children = node["overrides"]["children"];
		if (children) {
			std::vector<std::string_view> delKeys;
			for (auto childComp : children) {
				if (!childComp.second) {
					delKeys.push_back(childComp.first.Scalar());
					continue;
				}
				const YAML::Node& childOverrides = childComp.second["overrides"];
				if (!childOverrides || childOverrides.IsNull() || childOverrides.size() == 0) {
					delKeys.push_back(childComp.first.Scalar());
				}
			}
			for (int i = delKeys.size() - 1; i >= 0; --i) {
				children.remove(delKeys[i]);
			}
			if (children.size() == 0) {
				node["overrides"].remove("children");
			}
			if (node["overrides"].size() == 0) {
				node.remove("overrides");
			}
		}
		for (auto childNode : node) {
			if (!childNode.second || !childNode.second.IsMap()) {
				continue;
			}
			pendingNodes.push_back(childNode.second);
		}
	}
}

void SceneLoader::AddSpawnedComponent(const ScriptObject& obj, const YAML::Node& desc)
{
	assert(obj.GetRaw());
	if (spawnedComponents.find(obj.GetRaw()) != spawnedComponents.end()) {
		// log error
		return;
	}
 	spawnedComponents[obj.GetRaw()] = desc;
}

void SceneLoader::RemoveSpawnedComponent(SceneTree::Handle handle)
{
	if (!gSceneTree->IsValidHandle(handle)) {
		// log error
		return;
	}
	const SceneTree::Entity& entity = gSceneTree->Get(handle);
	if (!gSceneTree->IsValidHandle(entity.parent)) {
		// log error
		return;
	}
	const SceneTree::Entity& parent = gSceneTree->Get(entity.parent);
	assert(parent.obj.GetRaw());
	const auto parentIter = spawnedComponents.find(parent.obj.GetRaw());
	if (parentIter == spawnedComponents.end()) {
		// log error
		return;
	}
	YAML::Node parentNode = parentIter->second["overrides"]["children"];
	const std::string name = !entity.name.starts_with('$') ? '$' + entity.name : entity.name;
	if (!parentNode || !parentNode[name]) {
		// log error
		return;
	}
	parentNode.remove(name);
	assert(entity.obj.GetRaw());
	const auto iter = spawnedComponents.find(entity.obj.GetRaw());
	if (iter == spawnedComponents.end()) {
		// log error
		return;
	}
	spawnedComponents.erase(iter);
	std::vector<SceneTree::Handle> descendants;
	descendants.push_back(handle);
	while (!descendants.empty()) {
		SceneTree::Handle curHandle = descendants.back();
		descendants.pop_back();
		if (!gSceneTree->IsValidHandle(curHandle)) {
			// log error
			continue;
		}
		const SceneTree::Entity& curEntity = gSceneTree->Get(curHandle);
		for (SceneTree::Handle childHandle : curEntity.children) {
			if (!gSceneTree->IsValidHandle(childHandle)) {
				// log error
				continue;
			}
			const SceneTree::Entity& childEntity = gSceneTree->Get(childHandle);
			const auto childIter = spawnedComponents.find(childEntity.obj.GetRaw());
			if (childIter == spawnedComponents.end()) {
				// log error
				return;
			}
			spawnedComponents.erase(childIter);
			if (childEntity.children.empty()) {
				continue;
			}
			descendants.push_back(childHandle);
		}
	}
}

std::optional<YAML::Node> SceneLoader::FindSpawnedComponent(const ScriptObject& obj)
{
	assert(obj.GetRaw());
	const auto iter = spawnedComponents.find(obj.GetRaw());
	return iter != spawnedComponents.end() ? std::make_optional(iter->second) : std::nullopt;
}

void SceneLoader::UpdateSpawnedComponentTransform(SceneTree::Handle handle)
{
	if (!gSceneTree->IsValidHandle(handle)) {
		// log error
		return;
	}
	const SceneTree::Entity& entity = gSceneTree->Get(handle);
	const Math::Transform& transform = SharedStorage::Instance().transforms.AccessRead(entity.transform);
	std::optional<YAML::Node> sceneNodeOpt = FindSpawnedComponent(entity.obj);
	if (!sceneNodeOpt) {
		// log errror
		return;
	}

	YAML::Node& sceneNode = (*sceneNodeOpt)["overrides"]["transform"] = YAML::Node();
	const Math::Vector3 localPos = transform.GetLocalPosition();
	if (localPos != Math::Vector3::Zero) {
		YAML::Node& posNode = sceneNode["position"] = YAML::Node();
		posNode["x"] = localPos.x;
		posNode["y"] = localPos.y;
		posNode["z"] = localPos.z;
	}
	const Math::Quaternion localRot = transform.GetLocalRotation();
	if (localRot != Math::Quaternion::Identity) {
		// Computes rotation about y-axis (y), then x-axis (x), then z-axis (z)
		const Math::Vector3 euler = Math::RadToDeg(localRot.ToEuler());
		YAML::Node& rotNode = sceneNode["rotation"] = YAML::Node();
		rotNode["x"] = euler.y;
		rotNode["y"] = euler.x;
		rotNode["z"] = euler.z;
	}
	const Math::Vector3 localScale = transform.GetLocalScale();
	if (localScale != Math::Vector3(1.0f)) {
		YAML::Node& scaleNode = sceneNode["scale"] = YAML::Node();
		scaleNode["x"] = localScale.x;
		scaleNode["y"] = localScale.y;
		scaleNode["z"] = localScale.z;
	}
}
