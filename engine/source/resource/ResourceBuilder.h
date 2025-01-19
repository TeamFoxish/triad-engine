#pragma once

#include "Res.h"

class ResourceBuilder {
public:
    static YAML::Node BuildEmpty(const std::string_view type);
};
