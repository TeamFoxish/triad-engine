#include "ScriptObject.h"
#include "scripts/ScriptSystem.h"
#include "scriptdictionary.h"
#include "logs/Logs.h"
#include <string>
#include <iostream>
#include "scene/SceneLoader.h"
#include "shared/ResourceHandle.h"
#include "scripts/ComponentLoader.h"
#include "game/PrefabLoader.h"

#include <scripthelper.h>

ScriptObject::ScriptObject(const std::string& module, const std::string& typeDecl, ArgsT&& args) {
    asIScriptEngine* engine = gScriptSys->GetRawEngine();
    asIScriptModule* _module = engine->GetModule(module.c_str());
    if (!_module) {
        LOG_ERROR("Failed to find script module \"{}\"", module);
        return;
    }
    asITypeInfo* type = _module->GetTypeInfoByDecl(typeDecl.c_str());
    if (!type) {
        LOG_ERROR("Failed to find script type \"{}\"", typeDecl);
        return;
    }
    Init(type, std::move(args));
}

ScriptObject::ScriptObject(asITypeInfo* type, ArgsT&& args, asIScriptFunction* factory)
{
    Init(type, std::move(args), factory);
}

// TODO check ref count
ScriptObject::ScriptObject(asIScriptObject *object)
{
    asIScriptEngine* engine = gScriptSys->GetRawEngine();
    _type = engine->GetTypeInfoById(object->GetTypeId());
    _object = object;
    int fieldsCount = _object->GetPropertyCount();
    for (int i = 0; i < fieldsCount; i++) {
        _fields[_object->GetPropertyName(i)] = i;
    }
}

ScriptObject::ScriptObject(const ScriptObject& other) : 
    _object(other._object),
    _fields(other._fields),
    _type(other._type)
{}

ScriptObject::ScriptObject(ScriptObject&& other) noexcept : 
    _object(std::exchange(other._object, nullptr)),
    _fields(std::move(other._fields)),
    _type(std::exchange(other._type, nullptr))
{}

ScriptObject::~ScriptObject()
{}

ScriptObject& ScriptObject::operator=(const ScriptObject &other)
{
    if (this == &other)
        return *this;

    ScriptObject temp(other);
    std::swap(_object, temp._object);
    std::swap(_fields, temp._fields);
    std::swap(_type, temp._type);

    return *this;
}

ScriptObject& ScriptObject::operator=(ScriptObject &&other) noexcept
{
    ScriptObject temp(std::move(other));
    std::swap(_object, temp._object);
    std::swap(_fields, temp._fields);
    std::swap(_type, temp._type);
    return *this;

}

void ScriptObject::Init(asITypeInfo* type, ArgsT&& args, asIScriptFunction* factory)
{
    _type = type;
    if (args.empty() && !factory) {
        // TODO: check if default constructor exists
        asIScriptEngine* engine = gScriptSys->GetRawEngine();
        _object = static_cast<asIScriptObject*>(engine->CreateScriptObject(_type));
    } else {
        const size_t argsNum = args.size();
        _object = Construct(std::move(args), factory);
        if (_object == nullptr) {
            return;
        }
    }
    if (_type->GetTypeId() & asTYPEID_APPOBJECT) {
        return; // TEMP
    }
    int fieldsCount = _object->GetPropertyCount();
    for (int i = 0; i < fieldsCount; i++) {
        _fields[_object->GetPropertyName(i)] = i;
    }
}

asIScriptObject* ScriptObject::Construct(ArgsT&& args, asIScriptFunction* factory)
{
    if (!factory) {
        factory = FindAppropriateFactory(args);
        if (!factory) {
            LOG_ERROR("failed to construct object with type {}. no appropriate constructor was found with {} arguments", _type->GetName(), args.size());
            return nullptr;
        }
    }
    int rc = -1;

    asIScriptContext* ctx = gScriptSys->GetContext();

    // push state to reuse active context (allow nested calls)
    bool nested = (ctx->PushState() == asSUCCESS);

    // push constructor to context
    rc = ctx->Prepare(factory);
    if (rc < 0) {
        LOG_ERROR("failed to construct object \"{}\". failed to preapre script context. error code: {}", factory->GetName(), rc);
        if (nested) {
            ctx->PopState();
        }
        return nullptr;
    }

    // set constructor arguments
    const asUINT paramsCount = factory->GetParamCount();
    for (asUINT j = 0; j < paramsCount; ++j) {
        auto& [_, argValue] = args[j];
        if (!argValue) {
            continue;
        }
        ctx->SetArgObject(j, argValue);
    }

    // construct object
    rc = ctx->Execute();
    if (rc != asEXECUTION_FINISHED) {
        LOG_ERROR(GetExceptionInfo(ctx, true));
        LOG_ERROR("failed to construct object \"{}\". Exception on line number \"{}\": \"{}\"", 
            ctx->GetExceptionFunction()->GetName(),
            ctx->GetExceptionLineNumber(),
            ctx->GetExceptionString());
        if (nested) {
            ctx->PopState();
        }
        return nullptr;
    }

    asIScriptObject* retVal = *(asIScriptObject**)ctx->GetAddressOfReturnValue();
    if (nested) {
        ctx->PopState();
    }
    return retVal;
}

asIScriptFunction* ScriptObject::FindAppropriateFactory(const ArgsT& args)
{
    const asUINT factoriesCount = _type->GetFactoryCount();
    for (asUINT i = 0; i < factoriesCount; ++i) {
        asIScriptFunction* factory = _type->GetFactoryByIndex(i);
        const asUINT paramsCount = factory->GetParamCount();
        if (paramsCount != args.size()) {
            continue;
        }

        // compare parameter and passed argument types
        asUINT paramIdx = 0;
        for (paramIdx; paramIdx < paramsCount; ++paramIdx) {
            TypeId paramTypeId = -1;
            factory->GetParam(paramIdx, &paramTypeId);
            const TypeId argTypeId = args[paramIdx].first;
            if ((paramTypeId & argTypeId) == 0) {
                break;
            }
        }
        if (paramIdx < paramsCount) {
            continue; // inappropriate constructor
        }
        return factory;
    }
    return nullptr;
}

void ScriptObject::SetField(const std::string &name, void *value)
{
    const auto iter = _fields.find(name);
    if (iter == _fields.end()) {
        LOG_WARN("failed to set field value for object of type `{}`. no field `{}` was found", name, _type->GetName());
        return;
    }
    const asUINT fieldNumber = iter->second;
    void* currentValuePointer = _object->GetAddressOfProperty(fieldNumber);
    asUINT fieldType = _object->GetPropertyTypeId(fieldNumber);
    switch (fieldType) {
        case asTYPEID_VOID:
        {
            currentValuePointer = nullptr;
            break;
        }
        case asTYPEID_BOOL:
        {
            bool* newValueBool = static_cast<bool*>(value);
            bool* currentValueBool = static_cast<bool*>(currentValuePointer);
            *currentValueBool = *newValueBool;
            break;
        }
        case asTYPEID_INT8:
        {
            int8_t* newValueInt8 = static_cast<int8_t*>(value);
            int8_t* currentValueInt8 = static_cast<int8_t*>(currentValuePointer);
            *currentValueInt8 = *newValueInt8;
            break;
        }
        case asTYPEID_INT16:
        {
            int16_t* newValueInt16 = static_cast<int16_t*>(value);
            int16_t* currentValueInt16 = static_cast<int16_t*>(currentValuePointer);
            *currentValueInt16 = *newValueInt16;
            break;
        }
        case asTYPEID_INT32:
        {
            int32_t* newValueInt32 = static_cast<int32_t*>(value);
            int32_t* currentValueInt32 = static_cast<int32_t*>(currentValuePointer);
            *currentValueInt32 = *newValueInt32;
            break;
        }
        case asTYPEID_INT64:
        {
            int64_t* newValueInt64 = static_cast<int64_t*>(value);
            int64_t* currentValueInt64 = static_cast<int64_t*>(currentValuePointer);
            *currentValueInt64 = *newValueInt64;
            break;
        }
        case asTYPEID_UINT8:
        {
            uint8_t* newValueUint8 = static_cast<uint8_t*>(value);
            uint8_t* currentValueUint8 = static_cast<uint8_t*>(currentValuePointer);
            *currentValueUint8 = *newValueUint8;
            break;
        }
        case asTYPEID_UINT16:
        {
            uint16_t* newValueUint16 = static_cast<uint16_t*>(value);
            uint16_t* currentValueUint16 = static_cast<uint16_t*>(currentValuePointer);
            *currentValueUint16 = *newValueUint16;
            break;
        }
        case asTYPEID_UINT32:
        {
            uint32_t* newValueUint32 = static_cast<uint32_t*>(value);
            uint32_t* currentValueUint32 = static_cast<uint32_t*>(currentValuePointer);
            *currentValueUint32 = *newValueUint32;
            break;
        }
        case asTYPEID_UINT64:
        {
            uint64_t* newValueUint64 = static_cast<uint64_t*>(value);
            uint64_t* currentValueUint64 = static_cast<uint64_t*>(currentValuePointer);
            *currentValueUint64 = *newValueUint64;
            break;
        }
        case asTYPEID_FLOAT:
        {
            float* newValueFloat = static_cast<float*>(value);
            float* currentValueFloat = static_cast<float*>(currentValuePointer);
            *currentValueFloat = *newValueFloat;
            break;
        }
        case asTYPEID_DOUBLE:
        {
            double* newValueDouble = static_cast<double*>(value);
            double* currentValueDouble = static_cast<double*>(currentValuePointer);
            *currentValueDouble = *newValueDouble;
            break;
        }
        default:
        {
            if (fieldType == gScriptSys->GetStringType()->GetTypeId()) {
                std::string* newValueString = static_cast<std::string*>(value);
                std::string* currentValueString = static_cast<std::string*>(currentValuePointer);
                *currentValueString = *newValueString;
                break;
            }
            asITypeInfo* ti = gScriptSys->GetRawEngine()->GetTypeInfoById(fieldType);
            std::string name = ti->GetName();
            if (name == "array") {
                CScriptArray* newValueArray = static_cast<CScriptArray*>(value);
                CScriptArray* currentValueArray = static_cast<CScriptArray*>(currentValuePointer);
                *currentValueArray = *newValueArray;
                break;
            }
            if (name == "dictionary") {
                CScriptDictionary* newValueDictionary = static_cast<CScriptDictionary*>(value);
                CScriptDictionary* currentValueDictionary = static_cast<CScriptDictionary*>(currentValuePointer);
                *currentValueDictionary = *newValueDictionary;
                break;
            }
            if ((fieldType & asTYPEID_OBJHANDLE) > 0) {
                asIScriptObject* newValue = static_cast<asIScriptObject*>(value);
                asIScriptObject** currentValueObject = static_cast<asIScriptObject**>(currentValuePointer);
                *currentValueObject = newValue;
                newValue->AddRef();
                break;
            } else {
                // TODO: call AssignField here?? should trigger opAssign
                asIScriptObject* newValue = static_cast<asIScriptObject*>(value);
                asIScriptObject* currentValueObject = static_cast<asIScriptObject*>(currentValuePointer);
                *currentValueObject = *newValue;
                break;
            }
        }
    }
}

void ScriptObject::SetField(const std::string &name, const std::string& value)
{
    int fieldNumber = _fields[name];
    void* currentValuePointer = _object->GetAddressOfProperty(fieldNumber);
    asUINT fieldType = _object->GetPropertyTypeId(fieldNumber);
    switch (fieldType) {
        case asTYPEID_VOID:
        {
            currentValuePointer = nullptr;
            break;
        }
        case asTYPEID_BOOL:
        {
            bool* currentValueBool = static_cast<bool*>(currentValuePointer);
            if (value == "true") {
                *currentValueBool = true;
            } else {
                *currentValueBool = false;
            }
            break;
        }
        case asTYPEID_INT8:
        {
            int8_t* currentValueInt8 = static_cast<int8_t*>(currentValuePointer);
            *currentValueInt8 = std::stoi(value);
            break;
        }
        case asTYPEID_INT16:
        {
            int16_t* currentValueInt16 = static_cast<int16_t*>(currentValuePointer);
            *currentValueInt16 = std::stoi(value);
            break;
        }
        case asTYPEID_INT32:
        {
            int32_t* currentValueInt32 = static_cast<int32_t*>(currentValuePointer);
            *currentValueInt32 = std::stol(value);
            break;
        }
        case asTYPEID_INT64:
        {
            int64_t* currentValueInt64 = static_cast<int64_t*>(currentValuePointer);
            *currentValueInt64 = std::stoll(value);
            break;
        }
        case asTYPEID_UINT8:
        {
            uint8_t* currentValueUint8 = static_cast<uint8_t*>(currentValuePointer);
             uint32_t parsedULongForUint8 = std::stoul(value);
            assert (parsedULongForUint8 < std::numeric_limits<uint8_t>::max()
                    && parsedULongForUint8 > std::numeric_limits<uint8_t>::min());
            *currentValueUint8 = static_cast<uint8_t>(parsedULongForUint8);
            break;
        }
        case asTYPEID_UINT16:
        {
            uint16_t* currentValueUint16 = static_cast<uint16_t*>(currentValuePointer);
            uint32_t parsedULongForUint16 = std::stoul(value);
            assert (parsedULongForUint16 < std::numeric_limits<uint16_t>::max()
                    && parsedULongForUint16 > std::numeric_limits<uint16_t>::min());
            *currentValueUint16 = static_cast<uint16_t>(parsedULongForUint16);
            break;
        }
        case asTYPEID_UINT32:
        {
            uint32_t* currentValueUint32 = static_cast<uint32_t*>(currentValuePointer);
            *currentValueUint32 = std::stoul(value);
            break;
        }
        case asTYPEID_UINT64:
        {
            uint64_t* currentValueUint64 = static_cast<uint64_t*>(currentValuePointer);
            *currentValueUint64 = std::stoull(value);
            break;
        }
        case asTYPEID_FLOAT:
        {
            float* currentValueFloat = static_cast<float*>(currentValuePointer);
            *currentValueFloat = std::stof(value);
            break;
        }
        case asTYPEID_DOUBLE:
        {
            double* currentValueDouble = static_cast<double*>(currentValuePointer);
            *currentValueDouble = std::stod(value);
            break;
        }
        default:
        {
            if (fieldType == gScriptSys->GetStringType()->GetTypeId()) {
                std::string* currentValueString = static_cast<std::string*>(currentValuePointer);
                *currentValueString = value;
                break;
            }
        }
    }
}

void ScriptObject::SetArrayValue(CScriptArray* array, asUINT index, const std::string &value)
{
    asUINT fieldType = array->GetElementTypeId();
    switch (fieldType) {
        case asTYPEID_VOID:
        {
            // do nothing? i guess...
            break;
        }
        case asTYPEID_BOOL:
        {
            bool boolVal = false;
            if (value == "true") {
                boolVal = true;
            }
            array->SetValue(index, &boolVal);
            break;
        }
        case asTYPEID_INT8:
        {
            int8_t int8Val = std::stoi(value);
            array->SetValue(index, &int8Val);
            break;
        }
        case asTYPEID_INT16:
        {
            int16_t int16Val = std::stoi(value);
            array->SetValue(index, &int16Val);
            break;
        }
        case asTYPEID_INT32:
        {
            int32_t int32Val = std::stol(value);
            array->SetValue(index, &int32Val);
            break;
        }
        case asTYPEID_INT64:
        {
            int64_t int64Val = std::stoll(value);
            array->SetValue(index, &int64Val);
            break;
        }
        case asTYPEID_UINT8:
        {
            uint32_t parsedULongForUint8 = std::stoul(value);
            assert (parsedULongForUint8 < std::numeric_limits<uint8_t>::max()
                    && parsedULongForUint8 > std::numeric_limits<uint8_t>::min());
            uint8_t uint8Val = static_cast<uint8_t>(parsedULongForUint8);
            array->SetValue(index, &uint8Val);
            break;
        }
        case asTYPEID_UINT16:
        {
            uint32_t parsedULongForUint16 = std::stoul(value);
            assert (parsedULongForUint16 < std::numeric_limits<uint16_t>::max()
                    && parsedULongForUint16 > std::numeric_limits<uint16_t>::min());
            uint16_t uint16Val = static_cast<uint16_t>(parsedULongForUint16);
            array->SetValue(index, &uint16Val);
            break;
        }
        case asTYPEID_UINT32:
        {
            uint32_t uint32Val = std::stoul(value);
            array->SetValue(index, &uint32Val);
            break;
        }
        case asTYPEID_UINT64:
        {
            uint64_t uint64Val = std::stoull(value);
            array->SetValue(index, &uint64Val);
            break;
        }
        case asTYPEID_FLOAT:
        {
            float floatVal = std::stof(value);
            array->SetValue(index, &floatVal);
            break;
        }
        case asTYPEID_DOUBLE:
        {
            double doubleVal = std::stod(value);
            array->SetValue(index, &doubleVal);
            break;
        }
        default:
        {
            if (fieldType == gScriptSys->GetStringType()->GetTypeId()) {
                std::string stringVal = value;
                array->SetValue(index, &stringVal);
            }
        }
    }
}

void *ScriptObject::GetField(std::string name)
{
    return _object->GetAddressOfProperty(_fields[name]);
}

void ScriptObject::AssignField(const std::string& name, void* value)
{
    const int fieldIdx = _fields[name];
    void* currentValuePointer = _object->GetAddressOfProperty(fieldIdx);
    if (!currentValuePointer) {
        LOG_ERROR("Unable to assign object value for field {} with type {}. target field value was null", name, _type->GetName());
        return;
    }
    asIScriptEngine* engine = gScriptSys->GetRawEngine();
    const int rc = engine->AssignScriptObject(currentValuePointer, value, _type);
    if (rc < 0) {
        LOG_ERROR("Unable to assign object value for field {} with type {}. target field value was null", name, _type->GetName());
    }
}

const std::string ScriptObject::GetComponentPath() {
    std::string path = *static_cast<std::string*>(GetField("name"));
    asIScriptObject* parentObj = *static_cast<asIScriptObject**>(GetField("parent"));
    while (parentObj) {
        ScriptObject parent = ScriptObject(parentObj);
        std::string parentName = *static_cast<std::string*>(parent.GetField("name"));
        path = parentName + "/" + path;
        parentObj = *static_cast<asIScriptObject**>(GetField("parent"));
    }
    return path;
}

void ScriptObject::OverrideSimpleField(const std::string& fieldName, const std::string& value) {
    SetField(fieldName, value);
}

void ScriptObject::OverrideArray(const std::string& fieldName, const YAML::Node& node) {
    CScriptArray* array = static_cast<CScriptArray*>(GetField(fieldName));
    for (const auto& arrayOverride: node) {
        asUINT index = std::stoul(arrayOverride.first.Scalar().substr(1));
        // Assuming it's array of primitives or component refs
        if (arrayOverride.second.IsScalar()) {
            std::string arrayStringVal = arrayOverride.second.Scalar();
            // Assuming it's array of component refs
            if (arrayStringVal.starts_with("@")) {
                SceneLoader::AddArrayFieldToPendingState(this, fieldName, index, arrayStringVal.substr(1));
                continue;
            } else {
                // Assuming it's array of primitives
                SetArrayValue(array, index, arrayStringVal);
                continue;
            }
        } else {
             const int fieldTypeId = _object->GetPropertyTypeId(_fields[fieldName]);
            if ((fieldTypeId & asTYPEID_APPOBJECT) == asTYPEID_APPOBJECT) {
                // Native (App/cpp) object
                if ((fieldTypeId & asTYPEID_OBJHANDLE) == asTYPEID_OBJHANDLE) {
                    // Handle case
                    auto** obj = static_cast<CNativeObject**>(array->At(index));
                    (*obj)->ApplyOverrides(node);
                } else {
                    // Plain object case
                    auto* obj = static_cast<CNativeObject*>(array->At(index));
                    obj->ApplyOverrides(node);
                }
            } else {
                // Script object
                asIScriptObject* customObject;
                if ((fieldTypeId & asTYPEID_OBJHANDLE) == asTYPEID_OBJHANDLE) {
                    customObject = *static_cast<asIScriptObject**>(GetField(fieldName));
                } else {
                    customObject = static_cast<asIScriptObject*>(GetField(fieldName));
                }
                if (!customObject) {
                    // Object does not exists
                    asIScriptEngine* engine = gScriptSys->GetRawEngine();
                    asITypeInfo* customTypeInfo = engine->GetTypeInfoById(_object->GetPropertyTypeId(_fields[fieldName]));
                    customObject = static_cast<asIScriptObject*>(engine->CreateScriptObject(customTypeInfo));
                }
                ScriptObject object = ScriptObject(customObject);
                object.ApplyOverrides(node);
                // TODO maybe stupid
                array->SetValue(index, object.GetRaw());
            }
        }
    }
}

void ScriptObject::OverrideChildren(const YAML::Node& node) {
    CScriptArray* array = static_cast<CScriptArray*>(GetField("children"));
    for(const auto& arrayNode: node) {
        const std::string& childComponentName = arrayNode.first.Scalar().substr(1);
        for (asUINT i = 0; i < array->GetSize(); i++) {
            asIScriptObject* childRaw = *static_cast<asIScriptObject**>(array->At(i));
            if (childRaw == nullptr) {
                continue;
            }
            ScriptObject child = ScriptObject(childRaw);
            std::string* name = static_cast<std::string*>(child.GetField("name"));
            if (*name == childComponentName) {
                child.ApplyOverrides(arrayNode.second["overrides"]);
                return;
            }
        }
        if (arrayNode.second["prefab"]) {
            const YAML::Node parameters = arrayNode.second;
            YAML::Node prefabNode = parameters["prefab"];
            YAML::Node componentNode = parameters["component"];
            ResTag objectTag;
            if (prefabNode.IsDefined()) {
                objectTag = ResTag(ToStrid(prefabNode.Scalar()));
            }
            if (componentNode.IsDefined()) {
                objectTag = ResTag(ToStrid(componentNode.Scalar()));
            }
            ScriptObject* component = PrefabLoader::Create(objectTag, this);
            if (!component) {
                LOG_WARN("Failed to load component or prefab \"{}\"", objectTag.string());
                continue;
            }
            component->ApplyOverrides(parameters["overrides"]);
            component->SetField("name", childComponentName);
        }
        if (arrayNode.second["component"]) {
            const YAML::Node parameters = arrayNode.second;
            ResTag componentTag = ResTag(ToStrid(parameters["component"].Scalar()));
            ScriptObject* component = ComponentLoader::CreateComponent(componentTag, this);
            component->ApplyOverrides(parameters["overrides"]);
            component->SetField("name", childComponentName);
        }
    }
}

void ScriptObject::OverrideObject(const std::string& fieldName, const YAML::Node& node) {
    const auto iter = _fields.find(fieldName);
    if (iter == _fields.end()) {
        LOG_WARN("failed to override field for object with type `{}`. no field with name '{}' was found", _type->GetName(), fieldName);
        return;
    }
    const int fieldTypeId = _object->GetPropertyTypeId(_fields[fieldName]);

    if ((fieldTypeId & asTYPEID_APPOBJECT) == asTYPEID_APPOBJECT) {
        // Native (App/cpp) object
        OverrideNativeObject(fieldName, fieldTypeId, node);
    } else {
        // Script object
        asIScriptObject* customObject;
        if ((fieldTypeId & asTYPEID_OBJHANDLE) == asTYPEID_OBJHANDLE) {
            customObject = *static_cast<asIScriptObject**>(GetField(fieldName));
        } else {
            customObject = static_cast<asIScriptObject*>(GetField(fieldName));
        }
        if (!customObject) {
            // Object does not exists
            asIScriptEngine* engine = gScriptSys->GetRawEngine();
            asITypeInfo* customTypeInfo = engine->GetTypeInfoById(_object->GetPropertyTypeId(_fields[fieldName]));
            customObject = static_cast<asIScriptObject*>(engine->CreateScriptObject(customTypeInfo));
        }
        ScriptObject object = ScriptObject(customObject);
        object.ApplyOverrides(node);
        // TODO maybe stupid
        SetField(fieldName, object.GetRaw());
    }
}

void ScriptObject::OverrideNativeObject(const std::string& fieldName, const asUINT fieldType, const YAML::Node& node) {
    if ((fieldType & asTYPEID_OBJHANDLE) == asTYPEID_OBJHANDLE) {
        // Handle case
        auto** obj = static_cast<CNativeObject**>(GetField(fieldName));
        (*obj)->ApplyOverrides(node);
    } else {
        // Plain object case
        auto* obj = static_cast<CNativeObject*>(GetField(fieldName));
        obj->ApplyOverrides(node);
    }
}

void ScriptObject::OverrideResource(const std::string& fieldName, const YAML::Node& node) {
    // Parse value as resource handle
    // native type registered by application, so no reflection, sir :)
    auto* handle = static_cast<CResourceHandle*>(GetField(fieldName));
    handle->ApplyOverrides(node);
}

void ScriptObject::OverrideRef(const std::string& fieldName, const std::string& ref) {
    SceneLoader::AddFieldToPendingState(this, fieldName, ref.substr(1));
}

void ScriptObject::ApplyOverrides(const YAML::Node& overrides)
{
    asIScriptEngine* engine = gScriptSys->GetRawEngine();
    for (const auto& override : overrides) {
        const std::string& fieldName = override.first.Scalar();
        const YAML::Node& value = override.second;

        // Case scalar value
        if (value.IsScalar()) {
            const std::string& stringVal = override.second.Scalar();
            // Component ref
            if (stringVal.starts_with("@")) {
                OverrideRef(fieldName, stringVal);
                continue;
            }
            const int fieldTypeId = _object->GetPropertyTypeId(_fields[fieldName]);
            if (Triad::Resource::IsTag(stringVal)
                && (fieldTypeId & asTYPEID_APPOBJECT) == asTYPEID_APPOBJECT
                && fieldTypeId != gScriptSys->GetStringType()->GetTypeId()) {
                // Resource handle
                OverrideResource(fieldName, value);
                continue;
            } else {
                // Primitive value
                OverrideSimpleField(fieldName, stringVal);
                continue;
            }
            continue;
        }

        // Case map value
        if (value.IsMap()) {
            std::string firstFieldName = value.begin()->first.Scalar();
            if (firstFieldName.starts_with("@")) {
                // Array
                OverrideArray(fieldName, value);
                continue;
            }
            if (firstFieldName.starts_with("$")) {
                OverrideChildren(value);
                continue;
            }
            // Object override
            OverrideObject(fieldName, value);
            continue;
        }

        if (value.IsSequence()) {
            // not implemented yet
        }
    }
}
