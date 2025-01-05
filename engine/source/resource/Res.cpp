#include "Res.h"

#include "ResourceSystem.h"
#include <sstream>
#include <yaml-cpp/yaml.h>

namespace Triad {
ResPath Resource::GetTagSubpath(ResTag tag)
{
    const std::string_view tagStr = tag.string();
    assert(tagStr.size() > TAG_PREFIX.size());
    return tagStr.substr(TAG_PREFIX.size());
}

ResPath Triad::Resource::GetTagDynamicSubpath(ResTag tag)
{
    const std::string_view tagStr = tag.string();
    assert(tagStr.size() > TAG_DYNAMIC_PREFIX.size());
    return tagStr.substr(TAG_DYNAMIC_PREFIX.size());
}

bool Resource::IsTag(std::string_view str)
{
    return str.starts_with(TAG_PREFIX);
}

bool Resource::IsTagDynamic(std::string_view str)
{
    return str.starts_with(TAG_DYNAMIC_PREFIX);
}

ResTag Resource::MakeTag(ResPath subpath)
{
    std::stringstream ss;
    ss << TAG_PREFIX << subpath.string();
    return ToStrid(ss.str());
}

std::string_view Resource::GetFileTagSubpath(std::string_view filetag)
{
    assert(filetag.size() > FILE_PREFIX.size());
    return filetag.substr(FILE_PREFIX.size());;
}

bool Resource::IsFileTag(std::string_view filetag)
{
    return filetag.starts_with(FILE_PREFIX);
}

bool Resource::ResolveFileTagToFile(ResTag source, const YAML::Node& filetag, Triad::FileIO::FPath& path)
{
    if (!filetag.IsDefined() || !filetag.IsScalar()) {
        // TODO: add log
        return false;
    }
    return ResolveFileTagToFile(source, filetag.Scalar(), path);
}

bool Resource::ResolveFileTagToFile(ResTag source, std::string_view filetag, Triad::FileIO::FPath& path)
{
    using namespace Triad::FileIO;
    if (!IsFileTag(filetag)) {
        // TODO: add log
        return false;
    }
    ResPath srcPath;
    if (!gResourceSys->ResolveTagToFile(source, srcPath)) {
        // TODO: add log
        return false;
    }
    const FPath srcDir = srcPath.parent_path();
    assert(is_directory(srcDir));
    path = srcDir / GetFileTagSubpath(filetag);
    if (!IsFileExist(path)) {
        // TODO: add log
        return false;
    }
    return true;
}
}
