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
    void* GetField(std::string name);
    void AssignField(const std::string& name, void* value); // only for objects
    void ApplyOverrides(const YAML::Node& overrides);
    asIScriptObject* GetRaw() { return _object; };

private:
    asIScriptObject* Construct(ArgsT&& args = {});

private:
    std::unordered_map<std::string, asUINT> _fields;
    asITypeInfo* _type = nullptr;
    asIScriptObject* _object = nullptr;
};