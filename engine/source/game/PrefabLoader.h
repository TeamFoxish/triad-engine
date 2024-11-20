#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"

#include <unordered_map>
#include <functional>

class Component;

class PrefabLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, PrefabLoader> {
public:
	PrefabLoader();
	~PrefabLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override {}

	static Component* Create(ResTag tag, const YAML::Node& desc);

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<PrefabLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "prefab";
	}

private:
	static void TEMP_OverrideMaterial(class MeshComponent* mesh, const YAML::Node& overrides);

private:
	using CreateFunc = Component*(const YAML::Node&);
	static inline std::unordered_map<ResTag, std::function<CreateFunc>> creators;
};
