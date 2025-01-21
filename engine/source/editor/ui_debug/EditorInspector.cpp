#ifdef EDITOR

#include "EditorInspector.h"

#include "scripts/ScriptLoader.h"
#include "scripts/ScriptSystem.h"
#include "shared/Shared.h"
#include "logs/Logs.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#undef max
#undef min

static FieldAttributes operator|(FieldAttributes a, FieldAttributes b)
{
    return static_cast<FieldAttributes>(static_cast<int>(a) | static_cast<int>(b));
}
static FieldAttributes operator&(FieldAttributes a, FieldAttributes b)
{
    return static_cast<FieldAttributes>(static_cast<int>(a) & static_cast<int>(b));
}
static FieldAttributes& operator|= (FieldAttributes& a, FieldAttributes b) { return (FieldAttributes&)((int&)a |= (int)b); }
static FieldAttributes& operator&= (FieldAttributes& a, FieldAttributes b) { return (FieldAttributes&)((int&)a &= (int)b); }

static FieldAttributes FillFieldAttributes(const std::vector<std::string>& attributes)
{
    FieldAttributes flags = FieldAttributes::None;
    for (const std::string& attr : attributes) {
        switch (ToStrid(attr).hash_code()) {
            case "Show"_id:
                flags |= FieldAttributes::Show;
                break;
            case "Editable"_id:
                flags |= FieldAttributes::Show | FieldAttributes::Editable;
                break;
            default:
                LOG_ERROR("found unknown property attribute '{}'", attr);
        }
    }
    return flags;
}

bool EditorInspector::FieldBase::IsDisabled() const
{
    return (flags & FieldAttributes::Editable) == FieldAttributes::None;
}

struct FieldTypes {
    using FieldBase = EditorInspector::FieldBase;
    using FieldIdx = EditorInspector::FieldIdx;
    using ObjRaw = EditorInspector::ObjRaw;
    static constexpr float INDENT = EditorInspector::INDENT;

    static std::vector<std::unique_ptr<FieldBase>> GetObjectFields(const ScriptObject& scpObj)
    {
        std::vector<std::unique_ptr<FieldBase>> fields;
        ObjRaw* obj = scpObj.GetRaw();
        assert(obj);

        const int typeId = obj->GetTypeId();
        const asUINT propCount = obj->GetPropertyCount();
        for (asUINT idx = 0; idx < propCount; ++idx) {
            const std::string_view propName = obj->GetPropertyName(idx);
            const int propTypeId = obj->GetPropertyTypeId(idx);
            if (propTypeId < 0) {
                continue;
            }
            const std::vector<std::string> attributes =
                ScriptLoader::GetPropertyMetadata(typeId, idx);
            const FieldAttributes flags = FillFieldAttributes(attributes);
            if ((flags & FieldAttributes::Show) == FieldAttributes::None) {
                continue; // do not add field which is not shown
            }

            switch (propTypeId) {
                case asTYPEID_VOID: {
                    break;
                }
                case asTYPEID_BOOL: {
                    fields.push_back(std::make_unique<FieldTypes::FieldBool>(idx, flags));
                    break;
                }
                case asTYPEID_INT8: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<asINT8, ImGuiDataType_S8>>(idx, flags));
                    break;
                }
                case asTYPEID_INT16: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<asINT16, ImGuiDataType_S16>>(idx, flags));
                    break;
                }
                case asTYPEID_INT32: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<asINT32, ImGuiDataType_S32>>(idx, flags));
                    break;
                }
                case asTYPEID_INT64: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<asINT64, ImGuiDataType_S64>>(idx, flags));
                    break;
                }
                case asTYPEID_UINT8: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<asBYTE, ImGuiDataType_U8, FieldTypes::UnsignedDecimalFormat>>(idx, flags));
                    break;
                }
                case asTYPEID_UINT16: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<asWORD, ImGuiDataType_U16, FieldTypes::UnsignedDecimalFormat>>(idx, flags));
                    break;
                }
                case asTYPEID_UINT32: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<asUINT, ImGuiDataType_U32, FieldTypes::UnsignedDecimalFormat>>(idx, flags));
                    break;
                }
                case asTYPEID_UINT64: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<asQWORD, ImGuiDataType_U64, FieldTypes::UnsignedDecimalFormat>>(idx, flags));
                    break;
                }
                case asTYPEID_FLOAT: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<float, ImGuiDataType_Float, FieldTypes::FloatingFormat>>(idx, flags));
                    break;
                }
                case asTYPEID_DOUBLE: {
                    fields.push_back(std::make_unique<FieldTypes::FieldNumber<double, ImGuiDataType_Double, FieldTypes::FloatingFormat>>(idx, flags));
                    break;
                }
                default: {
                    const bool isHandle = (propTypeId & asTYPEID_OBJHANDLE) != 0 || (propTypeId & asTYPEID_HANDLETOCONST) != 0;
                    if ((propTypeId & asTYPEID_APPOBJECT) != 0) {
                        if (propTypeId == gScriptSys->GetStringType()->GetTypeId()) {
                            fields.push_back(std::make_unique<FieldTypes::FieldText>(idx, flags));
                            break;
                        }
                        const asITypeInfo* typeInfo = obj->GetEngine()->GetTypeInfoById(propTypeId);
                        if (typeInfo && CNativeObject::IsTypeNative(typeInfo)) {
                            // TODO: check if such cast ever works for asTYPEID_HANDLETOCONST
                            fields.push_back(std::make_unique<FieldTypes::FieldNativeObject>(idx, flags, isHandle));
                            break;
                        }
                    } else if ((propTypeId & asTYPEID_SCRIPTOBJECT) != 0) {
                        if (isHandle) {
                            // TODO
                            // component ref
                            // draw as string field?
                            break;
                        }
                        asIScriptObject* rawPtr = static_cast<asIScriptObject*>(obj->GetAddressOfProperty(idx));
                        if (!rawPtr) {
                            // log error
                            break;
                        }
                        fields.push_back(std::make_unique<FieldTypes::FieldObject>(idx, flags, std::string(propName), ScriptObject(rawPtr)));
                        break;
                    }
                    const asITypeInfo* entTypeInfo = obj->GetObjectType();
                    LOG_ERROR("failed to visualize field in the inspector. unsupported type found '{}' in '{}'", entTypeInfo->GetPropertyDeclaration(idx), entTypeInfo->GetName());
                }
            }
        }
        return fields;
    }

    class FieldBool : public FieldBase {
    public:
        FieldBool(FieldIdx idx, FieldAttributes flags)
            : FieldBase(idx, flags)
        {
        }

        virtual void Show(const ScriptObject& scpObj, const ScriptObject& compObj, std::vector<std::string_view>& path) override
        {
            ObjRaw* obj = scpObj.GetRaw();
            assert(obj);
            assert(fieldIdx < obj->GetPropertyCount());
            const char* name = obj->GetPropertyName(fieldIdx);
            void* ptr = obj->GetAddressOfProperty(fieldIdx);
            if (!ptr) {
                // log error
                return;
            }
            bool* val = static_cast<bool*>(ptr);
            if (Draw(name, val)) {
                UpdateSceneNodeValue(compObj, name, *val, path);
            }
        }

        static bool Draw(const char* name, bool* val) 
        {
            return ImGui::Checkbox(name, val);
        }
    };

    // TODO: use <inttypes.h> if ever needed
    struct SignedDecimalFormatT {
        static constexpr const char* format = "%d";
    };
    struct UnsignedDecimalFormat {
        static constexpr const char* format = "%u";
    };
    struct FloatingFormat {
        static constexpr const char* format = "%.3f";
    };
    template<typename ValT, ImGuiDataType DataT, typename FormatT = SignedDecimalFormatT, ImGuiInputTextFlags Flags = 0>
    class FieldNumber : public FieldBase {
    public:
        FieldNumber(FieldIdx idx, FieldAttributes flags)
            : FieldBase(idx, flags)
        {
        }

        virtual void Show(const ScriptObject& scpObj, const ScriptObject& compObj, std::vector<std::string_view>& path) override
        {
            ObjRaw* obj = scpObj.GetRaw();
            assert(obj);
            assert(fieldIdx < obj->GetPropertyCount());
            const char* name = obj->GetPropertyName(fieldIdx);
            void* ptr = obj->GetAddressOfProperty(fieldIdx);
            if (!ptr) {
                // log error
                return;
            }
            ValT* val = static_cast<ValT*>(ptr);
            if (Draw(name, val)) {
                UpdateSceneNodeValue(compObj, name, *val, path);
            }
        }

        static bool Draw(const char* name, ValT* val) 
        {
            return ImGui::DragScalar(name, DataT, val, 1.0f, nullptr, nullptr, FormatT::format, Flags);
        }
    };

    class FieldText : public FieldBase {
    public:
        FieldText(FieldIdx idx, FieldAttributes flags)
            : FieldBase(idx, flags)
        {
        }

        virtual void Show(const ScriptObject& scpObj, const ScriptObject& compObj, std::vector<std::string_view>& path) override
        {
            ObjRaw* obj = scpObj.GetRaw();
            assert(obj);
            assert(fieldIdx < obj->GetPropertyCount());
            const char* name = obj->GetPropertyName(fieldIdx);
            void* ptr = obj->GetAddressOfProperty(fieldIdx);
            if (!ptr) {
                // log error
                return;
            }
            std::string* str = static_cast<std::string*>(ptr);
            if (Draw(name, str)) {
                UpdateSceneNodeValue(compObj, name, *str, path);
            }
        }

        static bool Draw(const char* name, std::string* str) 
        {
            return ImGui::InputText(name, str);
        }
    };

    class FieldObject : public FieldBase {
    public:
        FieldObject(FieldIdx idx, FieldAttributes flags, const std::string& _fieldName, const ScriptObject& obj)
            : FieldBase(idx, flags)
            , fieldName(_fieldName)
            , fields(GetObjectFields(obj))
        {
        }

        virtual void Show(const ScriptObject& scpObj, const ScriptObject& compObj, std::vector<std::string_view>& path) override
        {
            // TODO: cache object from inspector instead of getting it again by idx???
            assert(scpObj.GetRaw());
            asIScriptObject* obj = static_cast<asIScriptObject*>(scpObj.GetRaw()->GetAddressOfProperty(fieldIdx));
            assert(obj && obj->GetObjectType());
            const std::string headerName = std::format("{} : {}", fieldName, obj->GetObjectType()->GetName());
            if (!ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                return;
            }
            ImGui::Indent(INDENT);
            path.emplace_back(fieldName);
            for (const auto& field : fields) {
                const bool isDisabled = field->IsDisabled();
                if (isDisabled) {
                    ImGui::BeginDisabled();
                }
                field->Show(ScriptObject(obj), compObj, path);
                if (isDisabled) {
                    ImGui::EndDisabled();
                }
            }
            path.pop_back();
            ImGui::Unindent(INDENT);
        }

    private:
        const std::string fieldName;
        const std::vector<std::unique_ptr<FieldBase>> fields;
    };

    class FieldNativeObject : public FieldBase {
    public:
        FieldNativeObject(FieldIdx idx, FieldAttributes flags, bool _isHandle)
            : FieldBase(idx, flags)
            , isHandle(_isHandle)
        {
        }

        virtual void Show(const ScriptObject& scpObj, const ScriptObject& compObj, std::vector<std::string_view>& path) override
        {
            ObjRaw* obj = scpObj.GetRaw();
            assert(obj);
            assert(fieldIdx < obj->GetPropertyCount());
            const char* name = obj->GetPropertyName(fieldIdx);
            void* ptr = obj->GetAddressOfProperty(fieldIdx);
            if (!ptr) {
                // log error
                return;
            }
            CNativeObject* nativeObj;
            if (isHandle) {
                nativeObj = *static_cast<CNativeObject**>(ptr);
            } else {
                nativeObj = static_cast<CNativeObject*>(ptr);
            }
            const int propTypeId = obj->GetPropertyTypeId(fieldIdx);
            if (propTypeId < 0) {
                // log error
                return;
            }
            const asITypeInfo* propTypeInfo = obj->GetEngine()->GetTypeInfoById(propTypeId);
            if (!propTypeInfo) {
                // log error
                return;
            }
            const std::string headerName = std::format("{} : {}", name, propTypeInfo->GetName());
            if (!ImGui::CollapsingHeader(headerName.c_str())) {
                return;
            }
            YAML::Node node = nativeObj->Serialize();
            if (DrawNested(node)) {
                nativeObj->ApplyOverrides(node);
                // TODO: check why native overrides force the whole object to be presented in scene desc
                UpdateSceneNodeValue(compObj, name, node, path);
            }
        }

        bool DrawNested(YAML::Node& node) {
            bool changed = false;
            ImGui::Indent(INDENT);
            for (auto iter = node.begin(); iter != node.end(); ++iter) {
                if (!iter->second || iter->second.IsNull()) {
                    continue;
                }
                if (iter->second.IsScalar()) {
                    changed |= DrawField(iter, node);
                } else if (iter->second.IsSequence()) {
                    // unsupported
                } else if (iter->first && iter->first.IsScalar() && iter->second.IsMap()) {
                    const std::string headerName = std::format("{} : Native", iter->first.Scalar());
                    if (!ImGui::CollapsingHeader(headerName.c_str())) {
                        continue;
                    }
                    changed |= DrawNested(iter->second);
                }
            }
            ImGui::Unindent(INDENT);
            return changed;
        }

        // https://github.com/jbeder/yaml-cpp/issues/973
        template<typename T>
        bool DecodeInto(const YAML::Node& node, T& object)
        {
            return node && YAML::convert<T>::decode(node, object);
        }

        bool DrawField(const YAML::iterator& iter, YAML::Node& parent) {
            assert(iter->first && iter->first.IsScalar());
            assert(iter->second.IsScalar());
            const std::string& key = iter->first.Scalar();
            {
                bool val;
                if (DecodeInto(iter->second, val)) {
                    const bool changed = FieldBool::Draw(key.c_str(), &val);
                    if (changed) {
                        parent[key] = val;
                    }
                    return changed;
                }
            }
            if (iter->second.Tag() == CNativeObject::INT_TAG) {
                // TODO: suport unsigned types?
                int64_t val;
                if (DecodeInto(iter->second, val)) {
                    const bool changed = FieldNumber<decltype(val), ImGuiDataType_S64>::Draw(key.c_str(), &val);
                    if (changed) {
                        parent[key] = val;
                    }
                    return changed;
                }
            }
            if (iter->second.Tag() == CNativeObject::STR_TAG) {
                std::string str;
                if (DecodeInto(iter->second, str)) {
                    const bool changed = FieldText::Draw(key.c_str(), &str);
                    if (changed) {
                        parent[key] = str;
                    }
                    return changed;
                }
            }
            {
                double val;
                if (DecodeInto(iter->second, val)) {
                    const bool changed = FieldNumber<decltype(val), ImGuiDataType_Double, FloatingFormat>::Draw(key.c_str(), &val);
                    if (changed) {
                        parent[key] = val;
                    }
                    return changed;
                }
            }
            
            LOG_ERROR("unsupported yaml scalar type found. or was i too lazy to support them all at once?");
            return false;
        }

    private:
        const bool isHandle = false;
    };
};

EditorInspector::EditorInspector(SceneTree::Handle entHandle)
{
    if (!gSceneTree->IsValidHandle(entHandle)) {
        // log error
        return;
    }
    const SceneTree::Entity& entity = gSceneTree->Get(entHandle);
    components.emplace_back(entHandle, entity);

    for (SceneTree::Handle childHandle : entity.children) {
        if (!gSceneTree->IsValidHandle(childHandle)) {
            // log error
            return;
        }
        const SceneTree::Entity& childEntity = gSceneTree->Get(childHandle);
        if (childEntity.isComposite) {
            continue;
        }
        components.emplace_back(childHandle, childEntity);
    }
}

void EditorInspector::Draw()
{
    ImGui::SeparatorText("Components");
    for (const auto& [handle, comp] : components) {
        if (!gSceneTree->IsValidHandle(handle)) {
            // log error
            continue;
        }
        comp.Show(gSceneTree->Get(handle));
    }
}

EditorInspector::Component::Component(const SceneTree::Entity& entity)
    : fields(FieldTypes::GetObjectFields(entity.obj))
{
}

void EditorInspector::Component::Show(const SceneTree::Entity& entity) const
{
    if (!entity.obj.GetTypeInfo()) {
        return;
    }
    const std::string headerName = std::format("{} : {}", entity.name, entity.obj.GetTypeInfo()->GetName());
    if (!ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    ImGui::Indent(INDENT);
    for (const auto& field : fields) {
        const bool isDisabled = field->IsDisabled();
        if (isDisabled) {
            ImGui::BeginDisabled();
        }
        std::vector<std::string_view> path;
        field->Show(entity.obj, entity.obj, path);
        if (isDisabled) {
            ImGui::EndDisabled();
        }
    }
    ImGui::Unindent(INDENT);
}

#endif // EDITOR
