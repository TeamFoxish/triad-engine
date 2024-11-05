#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"

#include <memory>
#include <unordered_map>

class Scene;

class SceneLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, SceneLoader> {
public:
	SceneLoader();
	~SceneLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override {}

	static std::unique_ptr<Scene> CreateScene(ResTag tag);
	static void FillScene(ResTag tag);

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<SceneLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "scene";
	}

private:
	static inline std::unordered_map<ResTag, const YAML::Node> sceneDescs;
};
