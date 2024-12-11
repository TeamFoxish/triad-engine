#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include "misc/Function.h"
#include <yaml-cpp/yaml.h>

namespace Triad {
namespace FileIO {
    using namespace std::filesystem;

    using FPath = path;
    using FException = std::ios_base::failure;

    inline bool IsFileExist(const FPath& path) { return exists(path); }

    std::string ReadFile(const FPath& path);

    YAML::Node ReadFileAsYaml(const FPath& path);

    void IterateDirectory(const FPath& path, BiConsumer<const std::string, const FPath> fileFunction);

    void IterateDirectory(const FPath& path, BiConsumer<const std::string, const FPath> fileFunction, bool recursive);

    void IterateDirectory(const FPath& path, BiConsumer<const YAML::Node, const FPath> fileFunction, const std::string& fileExtensions);

    void IterateDirectory(const FPath& path, BiConsumer<const YAML::Node, const FPath> fileFunction, bool recursive, const std::string& fileExtensions);
}
}
