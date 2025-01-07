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

ScriptObject* PrefabLoader::Create(ResTag tag, ScriptObject* parent)
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

	for (const auto& componentDesc : desc["components"]) {
		const std::string name = componentDesc.first.Scalar();
		const YAML::Node parameters = componentDesc.second;
		ResTag componentTag = ResTag(ToStrid(parameters["component"].Scalar()));
		ScriptObject* component = ComponentLoader::CreateComponent(componentTag, root);
		component->ApplyOverrides(parameters["overrides"]);
		component->SetField("name", name);
	}
	return root;
}
