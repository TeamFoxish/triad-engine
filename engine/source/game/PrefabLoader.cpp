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

ScriptObject* PrefabLoader::Create(ResTag* tag)
{
	const YAML::Node desc = _prefabs[*tag];

	// TEMP
	if (desc["class"] && desc["class"].Scalar() == "player") {
		auto player = new PlayerBall(gTempGame.get());
		player->Initialize();
		return nullptr;
	}

	asIScriptEngine* engine = gScriptSys->GetRawEngine();
	ScriptObject* root = new ScriptObject("Engine", "CompositeComponent");
	asITypeInfo* childArrayType = engine->GetTypeInfoByDecl("array<Component@>");
	CScriptArray* child = CScriptArray::Create(childArrayType);

	for (const auto& componentDesc : desc["components"]) {
		const std::string name = componentDesc.first.Scalar();
		const YAML::Node parameters = componentDesc.second;
		ResTag componentTag = ResTag(ToStrid(parameters["component"].Scalar()));
		ScriptObject* component = ComponentLoader::CreateComponent(&componentTag);
		component->ApplyOverrides(parameters["overrides"]);
		asIScriptObject* valObj = component->GetRaw();
		delete component;
		child->InsertLast(&valObj);
	}
	root->SetField("child", child);
	return root;
}
