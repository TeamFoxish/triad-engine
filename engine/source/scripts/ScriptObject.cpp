#include "ScriptObject.h"
#include "scripts/ScriptSystem.h"
#include "scriptdictionary.h"
#include "logs/Logs.h"
#include <string>
#include <iostream>
#include "scene/SceneLoader.h"
#include "shared/ResourceHandle.h"

ScriptObject::ScriptObject(const std::string& module, const std::string& typeDecl) {
    asIScriptEngine* engine = gScriptSys->GetRawEngine();
    asIScriptModule* _module = engine->GetModule(module.c_str());
    if (!_module) {
        LOG_ERROR("Failed to find script module \"{}\"", module);
    }
    _type = _module->GetTypeInfoByDecl(typeDecl.c_str());
    if (!_type) {
        LOG_ERROR("Failed to find script type \"{}\"", typeDecl);
    }
    // asIScriptModule* _module = engine->GetModule("Engine");
    // asUINT classCount = _module->GetTypedefCount();
    // for (asUINT i = 0; i < classCount; i++) {
    //     asITypeInfo* ti = _module->GetTypedefByIndex(i);
    //     LOG_INFO("Type: {} DECLARATION: {}", ti->GetTypeId(), ti->GetName());
    // }
    _object = static_cast<asIScriptObject*>(engine->CreateScriptObject(_type));
    int fieldsCount = _object->GetPropertyCount();
    for (int i = 0; i < fieldsCount; i++) {
        _fields[_object->GetPropertyName(i)] = i;
    }
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

ScriptObject::~ScriptObject()
{
}

void ScriptObject::SetField(const std::string &name, void *value)
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
            if (fieldType & asTYPEID_OBJHANDLE > 0) {
                asIScriptObject* newValue = static_cast<asIScriptObject*>(value);
                asIScriptObject** currentValueObject = static_cast<asIScriptObject**>(currentValuePointer);
                *currentValueObject = newValue;
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

void *ScriptObject::GetField(std::string name)
{
    return _object->GetAddressOfProperty(_fields[name]);
}

void ScriptObject::AssignField(const std::string& name, void* value)
{
    const int fieldIdx = _fields[name];
    void* currentValuePointer = _object->GetAddressOfProperty(fieldIdx);
    if (!currentValuePointer) {
        LOG_ERROR("unable to assign object value for field {} with type {}. target field value was null", name, _type->GetName());
        return;
    }
    asIScriptEngine* engine = gScriptSys->GetRawEngine();
    const int rc = engine->AssignScriptObject(currentValuePointer, value, _type);
    if (rc < 0) {
        LOG_ERROR("unable to assign object value for field {} with type {}. target field value was null", name, _type->GetName());
    }
}

void* parseOverrideValue(const std::string& val) {
    try {
        std::size_t pos;
        long long intResult = std::stoll(val, &pos);
        if (pos == val.size()) {
            return new int64_t(intResult);
        }
    } catch (const std::exception&) {
        // Not an integer
    }

    // Check if the string represents a floating-point number
    try {
        std::size_t pos;
        float floatResult = std::stof(val, &pos);
        if (pos == val.size()) {
            return new float(floatResult);
        }
    } catch (const std::exception&) {
        // Not a floating-point number
    }

    // Check if the string represents a double number
    try {
        std::size_t pos;
        double doubleResult = std::stod(val, &pos);
        if (pos == val.size()) {
            return new double(doubleResult);
        }
    } catch (const std::exception&) {
        // Not a double-point number
    }

    // Otherwise, treat it as a string
    return new std::string(val);
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
            // Assuming it's component ref, that will be linked in Link Pass
            if (stringVal.starts_with("@")) {
                SceneLoader::AddFieldToPendingState(this, fieldName, std::stoll(stringVal.substr(1)));
            } else {
                const int fieldTypeId = _object->GetPropertyTypeId(_fields[fieldName]);
                if (Triad::Resource::IsTag(stringVal) && (fieldTypeId & asTYPEID_APPOBJECT) == asTYPEID_APPOBJECT) {
                    // Parse value as resource handle
                    // native type registered by application, so no reflection, sir :)
                    auto* handle = static_cast<CResourceHandle*>(GetField(fieldName));
                    handle->ApplyOverrides(value);
                    return;
                } else {
                    // Assuming it's primitive value
                    void* val = parseOverrideValue(stringVal);
                    SetField(fieldName, val);
                    // if we set id for component than we add this component to registry fir Link Pass
                    if (fieldName == "id") {
                        SceneLoader::AddComponentToRegistry(*static_cast<uint64_t*>(val), this);
                    }
                }
            }
        }
        // Case map value
        if (value.IsMap()) {
            std::string firstFieldName = value.begin()->first.Scalar();
            // Assuming it's array override
            if (firstFieldName.starts_with("@")) {
                CScriptArray* array = static_cast<CScriptArray*>(GetField(fieldName));
                for (const auto& arrayOverride: value) {
                    uint64_t index = std::stoll(arrayOverride.first.Scalar().substr(1));
                    // Assuming it's array of primitives or component refs
                    if (arrayOverride.second.IsScalar()) {
                        std::string arrayStringVal = arrayOverride.second.Scalar();
                        // Assuming it's array of component refs
                        if (arrayStringVal.starts_with("@")) {
                            SceneLoader::AddArrayFieldToPendingState(this, fieldName, index, std::stoll(arrayStringVal.substr(1)));
                        } else {
                            // Assuming it's array of primitives
                            void* primitiveArrayVal = parseOverrideValue(arrayStringVal);
                            array->SetValue(index, primitiveArrayVal);
                        }
                    } else {
                        // Assuming it's array of custom types
                        asIScriptObject* arrayCustomObject = *static_cast<asIScriptObject**>(array->At(index));
                        // Object does not exists
                        if (!arrayCustomObject) {
                            arrayCustomObject = static_cast<asIScriptObject*>(engine->CreateUninitializedScriptObject(array->GetArrayObjectType()));
                        }
                        ScriptObject* arrayObject = new ScriptObject(arrayCustomObject);
                        arrayObject->ApplyOverrides(arrayOverride.second);
                        // TODO maybe stupid
                        asIScriptObject* raw = arrayObject->GetRaw();
                        array->SetValue(index, &raw);
                    }
                }
            } else {
                const int fieldTypeId = _object->GetPropertyTypeId(_fields[fieldName]);
                if ((fieldTypeId & asTYPEID_APPOBJECT) == asTYPEID_APPOBJECT) {
                    // native type registered by application, so no reflection, sir :)
                    auto* obj = static_cast<CNativeObject*>(GetField(fieldName));
                    obj->ApplyOverrides(value);
                    return;
                }
                
                // Assuming it's custom type field override
                asIScriptObject* customObject = static_cast<asIScriptObject*>(GetField(fieldName));
                // Object does not exists
                if (!customObject) {
                    asITypeInfo* customTypeInfo = engine->GetTypeInfoById(_object->GetPropertyTypeId(_fields[fieldName]));
                    customObject = static_cast<asIScriptObject*>(engine->CreateScriptObject(customTypeInfo));
                }
                ScriptObject object = ScriptObject(customObject);
                object.ApplyOverrides(override.second);
                // TODO maybe stupid
                SetField(fieldName, object.GetRaw());
            }
        }
        if (value.IsSequence()) {
            // not implemented yet
        }
    }
}
