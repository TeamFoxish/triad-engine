#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Triad {
namespace FileIO {
    using namespace std::filesystem;

    using FPath = path;
    using FException = std::ios_base::failure;

    inline bool IsFileExist(const FPath& path) { return exists(path); }

    std::string ReadFile(const FPath& path);
}
}
