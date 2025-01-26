#include "ComponentLoader.h"
#include "scripts/ScriptSystem.h"
#include "scene/SceneLoader.h"
#include "scene/SceneBindings.h"
#include "scripts/ScriptObject.h"
#include "logs/Logs.h"

ComponentLoader::ComponentLoader()
{
    FACTORY_INIT;
}

ComponentLoader::~ComponentLoader()
{
}

void ComponentLoader::Load(ResTag tag, const YAML::Node &desc)
{
    _components.emplace(tag, desc);
    LOG_INFO("Resource \"Component\" with tag \"{}\" was indexed", tag.string());
}

ScriptObject *ComponentLoader::CreateComponent(ResTag tag, ScriptObject* parent, const YAML::Node* sceneRepr)
{
    const YAML::Node& desc = _components[tag];
    ScriptObject::ArgsT args;
    args = {{asTYPEID_MASK_OBJECT, parent ? parent->GetRaw() : nullptr}};
    ScriptObject* component = new ScriptObject(desc["module"].Scalar(), desc["class"].Scalar(), std::move(args));
    cachedCompTags[component->GetRaw()] = tag;
    //if (defaultStates.find(tag) == defaultStates.end()) {
    //    // build default state of the object right after the constructor was called
    //    // utilized to determine which overrides should be dumped to yaml during component serialization
    //    defaultStates.insert({tag, component->BuildYaml(desc)}); // TODO: cache default state only in editor config?
    //}
    if (sceneRepr) {
        YAML::Node compNode = *sceneRepr;
        if (compNode.IsNull()) {
            PopulateEmptySceneYaml(compNode, tag);
        }
        SceneLoader::AddSpawnedComponent(*component, compNode);
    }
    component->ApplyOverrides(desc["overrides"]);
    return component;
}

void ComponentLoader::SetComponentName(ScriptObject& obj, const std::string& name)
{
    obj.SetField("name", name);
    const SceneTree::Handle handle = GetEntityHandleFromScriptObject(obj.GetRaw());
    if (!gSceneTree->IsValidHandle(handle)) {
        // TODO: log error
        return;
    }
    SceneTree::Entity& entity = gSceneTree->Get(handle);
    entity.name = name;
}

ResTag ComponentLoader::GetComponentTag(const ScriptObject& obj)
{
    assert(obj.GetRaw());
    const auto iter = cachedCompTags.find(obj.GetRaw());
    return iter != cachedCompTags.end() ? iter->second : ResTag{};
}

void ComponentLoader::AddComponentTag(const ScriptObject& obj, ResTag tag)
{
    assert(obj.GetRaw());
    cachedCompTags[obj.GetRaw()] = tag;
}

const YAML::Node& ComponentLoader::GetComponentDesc(ResTag tag)
{
    static YAML::Node emptyNode;
    const auto iter = _components.find(tag);
    return iter != _components.end() ? iter->second : emptyNode;
}

YAML::Node ComponentLoader::BuildCompYaml(SceneTree::Handle entHandle, const YAML::Node& origDesc)
{
    YAML::Node root;

    //const SceneTree::Entity& entity = gSceneTree->Get(entHandle);
    //ResTag tag = GetComponentTag(entity.obj);
    //if (tag == ResTag{}) {
    //    LOG_ERROR("failed to build component yaml for '{}'. unable to get cached component tag", entity.name);
    //    return root;
    //}
    //root["component"] = tag.string();
    //auto iter = defaultStates.find(tag);
    //if (iter == defaultStates.end()) {
    //    // unable to determine diff for component which was never created before using ComponentLoader::CreateComponent
    //    // the only possible way is to create a temporary compnent here, which is undesirable (for now at least)
    //    // it may occur when component has no separate .component file
    //    LOG_ERROR("failed to build component yaml '{}'. unable to locate its cached default state. the problem may be, that component has no separate .component file", entity.obj.GetTypeInfo()->GetName());
    //    return root;
    //}
    //const YAML::Node& defaultSt = iter->second;

    //const ScriptObject obj(entity.obj);
    //const YAML::Node node = obj.BuildYaml(origDesc);

    //if (!defaultSt || !node) {
    //    return root;
    //}

    //YAML::Node diff = Triad::Resource::DiffDescs(node, defaultSt);
    //if (diff.size() == 0) {
    //    return root;
    //}
    //root["overrides"] = std::move(diff);

    return root;
}

YAML::Node ComponentLoader::BuildOverridesList(SceneTree::Handle entHandle, const YAML::Node& origDesc)
{
    return YAML::Node();
}

void ComponentLoader::PopulateEmptySceneYaml(YAML::Node& prefabNode, ResTag tag)
{
    prefabNode["component"] = tag.string();
}
