#include "SceneLoader.h"

#include "Scene.h"
#include "game/PrefabLoader.h"
#include <yaml-cpp/yaml.h>

SceneLoader::SceneLoader()
{
	FACTORY_INIT;
}

SceneLoader::~SceneLoader()
{
}

void SceneLoader::Load(ResTag tag, const YAML::Node& desc)
{
	sceneDescs.emplace(tag, desc);
}

std::unique_ptr<Scene> SceneLoader::CreateScene(ResTag tag)
{
	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
	return scene;
}

void SceneLoader::FillScene(ResTag tag)
{
	const YAML::Node& desc = sceneDescs[tag];
	for (const auto& obj : desc["objects"]) {
		const YAML::Node& params = obj.second;
		if (params.IsScalar()) {
			const std::string val = params.Scalar();
			PrefabLoader::Create(ToStrid(val), {});
		}
		else {
			const std::string val = params["ref"].Scalar();
			PrefabLoader::Create(ToStrid(val), params["overrides"]);
		}
	}
}
