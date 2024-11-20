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
