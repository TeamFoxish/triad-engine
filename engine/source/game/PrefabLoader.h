#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"
#include "scripts/ScriptObject.h"
#include <unordered_map>
#include <functional>
#include "logs/Logs.h"

class Component;

class PrefabLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, PrefabLoader> {
public:
	PrefabLoader();
	~PrefabLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override {}

	static ScriptObject* Create(ResTag* tag, ScriptObject* parent = nullptr);

    static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<PrefabLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "prefab";
	}

private:
	static inline std::unordered_map<ResTag, const YAML::Node> _prefabs;
};
