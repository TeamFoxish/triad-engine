#pragma once

#include "misc/Strid.h"
#include "file/FileIO.h"

using ResTag = Strid;
using ResPath = Triad::FileIO::FPath;

namespace YAML {
	class Node;
}

namespace Triad {
struct Resource {
	static constexpr std::string_view TAG_PREFIX = "res://";
	static constexpr std::string_view TAG_DYNAMIC_PREFIX = "resd://";
	static constexpr std::string_view FILE_PREFIX = "file://";

	// TODO: replace ResPath with std::string_view
	static ResPath GetTagSubpath(ResTag tag);
	static ResPath GetTagDynamicSubpath(ResTag tag);
	static bool IsTag(std::string_view str);
	static bool IsTagDynamic(std::string_view str);
	static ResTag MakeTag(ResPath subpath);

	static std::string_view GetFileTagSubpath(std::string_view filetag);
	static bool IsFileTag(std::string_view filetag);
	
	static bool ResolveFileTagToFile(ResTag source, const YAML::Node& filetag, Triad::FileIO::FPath& path);
	static bool ResolveFileTagToFile(ResTag source, std::string_view filetag, Triad::FileIO::FPath& path);
};
}
