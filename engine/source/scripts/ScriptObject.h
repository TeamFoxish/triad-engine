#pragma once

#include "angelscript.h"
#include <string>
#include "scriptarray.h"
#include <unordered_map>
#include <yaml-cpp/yaml.h>

class ScriptObject {
public:
    ScriptObject(const std::string& module, const std::string& typeDecl);
    ScriptObject(asIScriptObject* object);
    ~ScriptObject();
    void SetField(const std::string& name, void* value);
    void* GetField(std::string name);
    void ApplyOverrides(const YAML::Node& overrides);
    asIScriptObject* GetRaw() { return _object; };

private:
    std::unordered_map<std::string, asUINT> _fields;
    asITypeInfo* _type;
    asIScriptObject* _object;
};