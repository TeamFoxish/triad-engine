#include "ComponentLoader.h"
#include "scripts/ScriptSystem.h"
#include "scene/SceneLoader.h"
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

ScriptObject *ComponentLoader::CreateComponent(ResTag *tag)
{
    const YAML::Node desc = _components[*tag];
    ScriptObject* component = new ScriptObject(desc["module"].Scalar(), desc["class"].Scalar());
    component->ApplyOverrides(desc["overrides"]);
    return component;
}
