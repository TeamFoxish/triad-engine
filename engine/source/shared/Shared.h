#pragma once

#include "resource/Res.h"

class CNativeObject {
public:
	virtual void ApplyOverrides(const YAML::Node& overrides) {}
};
