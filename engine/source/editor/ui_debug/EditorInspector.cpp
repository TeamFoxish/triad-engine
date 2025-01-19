#ifdef EDITOR

#include "EditorInspector.h"

#include "scripts/ScriptLoader.h"
#include "scripts/ScriptSystem.h"
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

    class FieldBool : public FieldBase {
    public:
        FieldBool(FieldIdx idx, FieldAttributes flags)
            : FieldBase(idx, flags)
        {
        }

        virtual void Show(const SceneTree::Entity& entity) override 
        {
            ObjRaw* obj = entity.obj.GetRaw();
            assert(obj);
            assert(fieldIdx < obj->GetPropertyCount());
            const char* name = obj->GetPropertyName(fieldIdx);
            void* ptr = obj->GetAddressOfProperty(fieldIdx);
            if (!ptr) {
                // log error
                return;
            }
            bool* val = static_cast<bool*>(ptr);
            if (ImGui::Checkbox(name, val)) {
                UpdateSceneNodeValue(entity, name, *val);
            }
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
        using Limits = std::numeric_limits<ValT>;

        FieldNumber(FieldIdx idx, FieldAttributes flags)
            : FieldBase(idx, flags)
        {
        }

        virtual void Show(const SceneTree::Entity& entity) override 
        {
            ObjRaw* obj = entity.obj.GetRaw();
            assert(obj);
            assert(fieldIdx < obj->GetPropertyCount());
            const char* name = obj->GetPropertyName(fieldIdx);
            void* ptr = obj->GetAddressOfProperty(fieldIdx);
            if (!ptr) {
                // log error
                return;
            }
            ValT* val = static_cast<ValT*>(ptr);
            if (ImGui::InputScalar(name, DataT, val, nullptr, nullptr, FormatT::format, Flags)) {
                UpdateSceneNodeValue(entity, name, *val);
            }
        }
    };

    class FieldText : public FieldBase {
    public:
        FieldText(FieldIdx idx, FieldAttributes flags)
            : FieldBase(idx, flags)
        {
        }

        virtual void Show(const SceneTree::Entity& entity) override 
        {
            ObjRaw* obj = entity.obj.GetRaw();
            assert(obj);
            assert(fieldIdx < obj->GetPropertyCount());
            const char* name = obj->GetPropertyName(fieldIdx);
            void* ptr = obj->GetAddressOfProperty(fieldIdx);
            if (!ptr) {
                // log error
                return;
            }
            std::string* str = static_cast<std::string*>(ptr);
            if (ImGui::InputText(name, str)) {
                UpdateSceneNodeValue(entity, name, *str);
            }
        }
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
{
    ObjRaw* obj = entity.obj.GetRaw();
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
                if ((propTypeId & asTYPEID_APPOBJECT) != 0) {
                    if (propTypeId == gScriptSys->GetStringType()->GetTypeId()) {
                        fields.push_back(std::make_unique<FieldTypes::FieldText>(idx, flags));
                        return;
                    }
                }
            }
        }
    }
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
    for (const auto& field : fields) {
        const bool isDisabled = field->IsDisabled();
        if (isDisabled) {
            ImGui::BeginDisabled();
        }
        field->Show(entity);
        if (isDisabled) {
            ImGui::EndDisabled();
        }
    }
}

#endif // EDITOR
