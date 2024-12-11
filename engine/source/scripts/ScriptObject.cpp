#include "ScriptObject.h"
#include "scripts/ScriptSystem.h"
#include "scriptdictionary.h"
#include "logs/Logs.h"
#include <string>
#include <iostream>

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
    _object = static_cast<asIScriptObject*>(engine->CreateUninitializedScriptObject(_type));
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
    _type->Release();
}

void ScriptObject::SetField(const std::string &name, void *value)
{
    int fieldNumber = _fields[name];
    void* currentValuePointer = _object->GetAddressOfProperty(fieldNumber);
    asUINT fieldType = _object->GetPropertyTypeId(fieldNumber);
     switch (fieldType) {
        case asTYPEID_VOID:
            currentValuePointer = nullptr;
            break;
        case asTYPEID_BOOL:
        {
            bool* newValueBool = static_cast<bool*>(value);
            bool* currentValueBool = static_cast<bool*>(currentValuePointer);
            *currentValueBool = *newValueBool;
        }
            break;
        case asTYPEID_INT8:
        {
            int8_t* newValueInt8 = static_cast<int8_t*>(value);
            int8_t* currentValueInt8 = static_cast<int8_t*>(currentValuePointer);
            *currentValueInt8 = *newValueInt8;
        }
            break;
        case asTYPEID_INT16:
        {
            int16_t* newValueInt16 = static_cast<int16_t*>(value);
            int16_t* currentValueInt16 = static_cast<int16_t*>(currentValuePointer);
            *currentValueInt16 = *newValueInt16;
        }
            break;
        case asTYPEID_INT32:
        {
            int32_t* newValueInt32 = static_cast<int32_t*>(value);
            int32_t* currentValueInt32 = static_cast<int32_t*>(currentValuePointer);
            *currentValueInt32 = *newValueInt32;
        }
            break;
        case asTYPEID_INT64:
        {
            int64_t* newValueInt64 = static_cast<int64_t*>(value);
            int64_t* currentValueInt64 = static_cast<int64_t*>(currentValuePointer);
            *currentValueInt64 = *newValueInt64;
        }
            break;
        case asTYPEID_UINT8:
        {
            uint8_t* newValueUint8 = static_cast<uint8_t*>(value);
            uint8_t* currentValueUint8 = static_cast<uint8_t*>(currentValuePointer);
            *currentValueUint8 = *newValueUint8;
        }
            break;
        case asTYPEID_UINT16:
        {
            uint16_t* newValueUint16 = static_cast<uint16_t*>(value);
            uint16_t* currentValueUint16 = static_cast<uint16_t*>(currentValuePointer);
            *currentValueUint16 = *newValueUint16;
        }
            break;
        case asTYPEID_UINT32:
        {
            uint32_t* newValueUint32 = static_cast<uint32_t*>(value);
            uint32_t* currentValueUint32 = static_cast<uint32_t*>(currentValuePointer);
            *currentValueUint32 = *newValueUint32;
        }
            break;
        case asTYPEID_UINT64:
        {
            uint64_t* newValueUint64 = static_cast<uint64_t*>(value);
            uint64_t* currentValueUint64 = static_cast<uint64_t*>(currentValuePointer);
            *currentValueUint64 = *newValueUint64;
        }
            break;
        case asTYPEID_FLOAT:
        {
            float* newValueFloat = static_cast<float*>(value);
            float* currentValueFloat = static_cast<float*>(currentValuePointer);
            *currentValueFloat = *newValueFloat;
        }
            break;
        case asTYPEID_DOUBLE:
        {
            double* newValueDouble = static_cast<double*>(value);
            double* currentValueDouble = static_cast<double*>(currentValuePointer);
            *currentValueDouble = *newValueDouble;
        }
            break;
        default:
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
            asIScriptObject* newValue = gScriptSys->CreateComponentHandle(static_cast<std::string*>(value));
            asIScriptObject* currentValueObject = static_cast<asIScriptObject*>(currentValuePointer);
            *currentValueObject = *newValue;
            break;
        }

}

void *ScriptObject::GetField(std::string name)
{
    return _object->GetAddressOfProperty(_fields[name]);
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
    for (const auto& override: overrides) {
        std::string fieldName = override.first.Scalar();
        YAML::Node value = override.second;
        if (value.IsScalar()) {
            void* val = parseOverrideValue(override.second.Scalar());
            SetField(fieldName, val);
            if (fieldName == "id") {
                gScriptSys->AddComponentToContext(this, *static_cast<uint64_t*>(val));
            }
        }
        if (value.IsMap()) {
            CScriptArray* child = static_cast<CScriptArray*>(GetField("child"));
            asITypeInfo* searchedType = engine->GetModule("Engine")->GetTypeInfoByDecl(fieldName.c_str());
            asIScriptObject* searchedObject = nullptr;
            int childSize = child->GetSize();
            for (int i = 0; i < childSize; i++) {
                asIScriptObject* childObject = *reinterpret_cast<asIScriptObject**>(child->At(i));
                if (childObject != nullptr) {
                    if ( childObject->GetTypeId() == searchedType->GetTypeId()) {
                        searchedObject = childObject;
                        break;
                    }
                }
            }
            if (searchedObject != nullptr) {
                ScriptObject children = ScriptObject(searchedObject);
                children.ApplyOverrides(override.second);
            }
        }
        if (value.IsSequence()) {
            
        }
    }
}
