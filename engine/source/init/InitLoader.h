#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"

class InitLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, InitLoader> {
public:
	InitLoader();
	~InitLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override {}

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<InitLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "init";
	}

	static inline ResTag startUpSceneTag;
};
