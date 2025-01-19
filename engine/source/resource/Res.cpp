#include "Res.h"

#include "ResourceSystem.h"
#include "logs/Logs.h"

#include <sstream>
#include <yaml-cpp/yaml.h>
#include <queue>

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


YAML::Node Resource::DiffDescs(const YAML::Node& modified, const YAML::Node& original)
{
    struct Entry {
        YAML::Node* parent; // where the key will be written
        const YAML::Node* key = nullptr;
        const YAML::Node* modified = nullptr;
        const YAML::Node* original = nullptr;
    };

    YAML::Node diff;

    // preserve order by using queue
    std::queue<Entry> pending;
    for (const auto& entry : modified) {
        assert(entry.first);
        const YAML::Node& key = entry.first;
        assert(key.IsScalar());
        const YAML::Node& origNode = original[key];
        const YAML::Node* orig = origNode ? &origNode : nullptr;
        pending.emplace(&diff, &entry.first, &entry.second, orig);
    }

    while (!pending.empty()) {
        const Entry entry = pending.front();
        pending.pop();
        YAML::Node& parent = *entry.parent;

        if (!entry.original || entry.modified->Type() != entry.original->Type()) {
            // entry wasn't spotted in original or yaml type was modified
            parent[entry.key->Scalar()] = *entry.modified;
            continue;
        }
        //if (entry.modified->Type() != entry.original->Type()) {
        //    // field values have different yaml types
        //    LOG_ERROR("found yaml nodes with same key (path), but different types. key '{}' with type '{}' in modified yaml and type '{}' in original", entry.key->Scalar(), (int)entry.modified->Type(), (int)entry.original->Type());
        //    continue;
        //}
        const YAML::NodeType::value fieldType = entry.modified->Type();
        switch (fieldType) {
            case YAML::NodeType::Undefined:
                assert(false); // shouldn't be possible
                break;
            case YAML::NodeType::Null:
                break; // ignore empty fields
            case YAML::NodeType::Scalar: {
                // compare string values
                if (entry.modified->Scalar() != entry.original->Scalar()) {
                    parent[entry.key->Scalar()] = entry.modified->Scalar();
                }
                break;
            }
            case YAML::NodeType::Sequence: {
                // check if should add the whole modified sequence
                if (!AreEqualSequences(*entry.modified, *entry.original)) {
                    parent[entry.key->Scalar()] = *entry.modified; // copy the whole modified sequence
                }
                break;
            }
            case YAML::NodeType::Map: {
                YAML::Node& map = parent[*entry.key] = {};
                for (const auto& node : *entry.modified) {
                    assert(node.first);
                    const YAML::Node& key = node.first;
                    assert(key.IsScalar());
                    const YAML::Node& origNode = (*entry.original)[key];
                    const YAML::Node* orig = origNode ? &origNode : nullptr;
                    pending.emplace(&map, &node.first, &node.second, orig);
                }
                break;
            }
            default:
                assert(false); // unsupported node type
        }
    }

    return diff;
}

bool Resource::AreEqualNodes(const YAML::Node& a, const YAML::Node& b)
{
    if (a.Type() != b.Type()) {
        return false;
    }

    const YAML::NodeType::value fieldType = a.Type();
    switch (fieldType) {
        case YAML::NodeType::Undefined:
        case YAML::NodeType::Null:
            return true;
        case YAML::NodeType::Scalar:
            // compare string values
            return a.Scalar() == b.Scalar();
        case YAML::NodeType::Sequence:
            return AreEqualSequences(a, b);
        case YAML::NodeType::Map:
            return AreEqualMaps(a, b);
        default:
            assert(false); // unsupported node type
            return false;
    }
}

bool Resource::AreEqualSequences(const YAML::Node& a, const YAML::Node& b)
{
    assert(a.IsSequence() && b.IsSequence());
    if (a.size() != b.size()) {
        return false;
    }
    const size_t length = a.size();
    for (int i = 0; i < length; ++i) {
        if (!AreEqualNodes(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

bool Resource::AreEqualMaps(const YAML::Node& a, const YAML::Node& b)
{
    assert(a.IsMap() && b.IsMap());
    if (a.size() != b.size()) {
        return false;
    }
    YAML::const_iterator aIt = a.begin();
    YAML::const_iterator bIt = b.begin();
    for (;aIt != a.end() && bIt != b.end(); ++aIt, ++bIt) {
        if (!AreEqualNodes(aIt->second, bIt->second)) {
            return false;
        }
    }
    return true;
}
}
