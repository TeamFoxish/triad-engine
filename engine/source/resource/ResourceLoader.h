#pragma once

#define YAML_CPP_STATIC_DEFINE

#include <yaml-cpp/yaml.h>
#include "ResourceSystem.h"

class ResourceLoader {
public:
	virtual ~ResourceLoader() = default;

	virtual void Load(ResTag tag, const YAML::Node& desc) = 0;
	virtual void Unload(ResTag tag) = 0;

protected:
	ResourceLoader() = default;
};
