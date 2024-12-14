#pragma once

#include "angelscript.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "scripts/ScriptObject.h"
#include "resource/ResourceLoader.h"
#include "misc/Factory.h"

class ComponentLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, ComponentLoader> {
public:
    ComponentLoader();
	~ComponentLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override {};

    static ScriptObject* CreateComponent(ResTag* tag, ScriptObject* parent = nullptr);

    static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<ComponentLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "component";
	}

private:
	static inline std::unordered_map<ResTag, const YAML::Node> _components;
};