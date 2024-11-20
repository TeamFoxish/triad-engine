#pragma once

#include <yaml-cpp/yaml.h>
#include "Res.h"

class ResourceLoader {
public:
	virtual ~ResourceLoader() = default;

	virtual void Load(ResTag tag, const YAML::Node& desc) = 0;
	virtual void Unload(ResTag tag) = 0;

protected:
	ResourceLoader() = default;
};
