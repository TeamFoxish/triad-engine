#include "ResourceBuilder.h"

#include <yaml-cpp/yaml.h>

YAML::Node ResourceBuilder::BuildEmpty(const std::string_view type)
{
    YAML::Node root;
    root["content"] = YAML::Node();
    root["type"] = type;
    return root;
}
