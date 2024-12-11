#include "FileIO.h"

std::string Triad::FileIO::ReadFile(const FPath& path)
{
    constexpr std::size_t READ_SIZE = 4096;
    std::ifstream stream(path);
    stream.exceptions(std::ios_base::badbit);
    if (not stream) {
        throw std::ios_base::failure("file doesn't exist");
    }
    std::stringstream res;
    std::string buf(READ_SIZE, '\0');
    while (stream.read(&buf[0], READ_SIZE)) {
        res << std::string_view(buf).substr(0, stream.gcount());
    }
    res << std::string_view(buf).substr(0, stream.gcount());
    return res.str();
}

YAML::Node Triad::FileIO::ReadFileAsYaml(const FPath &path)
{
    return YAML::LoadFile(path.generic_string());
}

void Triad::FileIO::IterateDirectory(const FPath &path, BiConsumer<std::string, const FPath> fileFunction)
{
    IterateDirectory(path, fileFunction, false);
}

void Triad::FileIO::IterateDirectory(const FPath &path, BiConsumer<std::string, const FPath> fileFunction, bool recursive)
{
    for (const auto& entry : directory_iterator(path)) {
        if (recursive && entry.is_directory()) {
            IterateDirectory(entry, fileFunction, recursive);
        } else {
            fileFunction(ReadFile(entry), entry);
        }
    }
}

void Triad::FileIO::IterateDirectory(const FPath &path, BiConsumer<YAML::Node, const FPath> fileFunction, const std::string& fileExtension)
{
    IterateDirectory(path, fileFunction, false, fileExtension);
}

void Triad::FileIO::IterateDirectory(const FPath &path, BiConsumer<YAML::Node, const FPath> fileFunction, bool recursive, const std::string& fileExtension)
{
    for (const auto& entry : directory_iterator(path)) {
        if (recursive && entry.is_directory()) {
            IterateDirectory(entry, fileFunction, recursive, fileExtension);
        } else {
            if (entry.path().extension() == fileExtension)
            fileFunction(ReadFileAsYaml(entry), entry);
        }
    }
}
