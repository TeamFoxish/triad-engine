#pragma once

#include "angelscript.h"
#include <string>
#include "scriptarray.h"
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

class ScriptObject {
public:
    using TypeId = int;
    using ArgsT = std::vector<std::pair<TypeId, void*>>; // should be implemented using varargs in future

    ScriptObject(const std::string& module, const std::string& typeDecl, ArgsT&& args = {});
    ScriptObject(asIScriptObject* object);
    ~ScriptObject();
    void SetField(const std::string& name, void* value);
    void SetField(const std::string &name, const std::string &value);
    void *ParseStringByType(const std::string &value, asUINT fieldType);
    void *GetField(std::string name);
    void AssignField(const std::string& name, void* value);
    // only for objects
    void ApplyOverrides(const YAML::Node& overrides);
    asIScriptObject* GetRaw() { return _object; };
    const std::string GetComponentPath();

private:
    asIScriptObject* Construct(ArgsT&& args = {});

    void OverrideObject(const std::string &fieldName, const YAML::Node &node);
    void OverrideNativeObject(const std::string &fieldName, const asUINT fieldType, const YAML::Node &node);
    void OverrideResource(const std::string &fieldName, const YAML::Node &node);
    void OverrideArray(const std::string &fieldName, const YAML::Node &node);
    void OverrideChildren(const YAML::Node &node);
    void OverrideRef(const std::string &fieldName, const std::string &ref);
    void OverrideSimpleField(const std::string &fieldName, const std::string &value);

private:
    std::unordered_map<std::string, asUINT> _fields;
    asITypeInfo* _type = nullptr;
    asIScriptObject* _object = nullptr;
};