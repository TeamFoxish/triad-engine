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
    ScriptObject(asITypeInfo* type, ArgsT&& args = {}, asIScriptFunction* factory = nullptr);
    explicit ScriptObject(asIScriptObject* object);
    ScriptObject(const ScriptObject& other);
    //ScriptObject(ScriptObject&& other) noexcept;
    ~ScriptObject();
    //ScriptObject& operator=(const ScriptObject& other);
    //ScriptObject& operator=(ScriptObject&& other) noexcept;
    void SetField(const std::string& name, void* value);
    void SetField(const std::string &name, const std::string &value);
    static void SetArrayValue(CScriptArray *array, asUINT index, const std::string &value);
    void *GetField(const std::string& name);
    void AssignField(const std::string& name, void* value);
    // only for objects
    void ApplyOverrides(const YAML::Node& overrides);
    asITypeInfo* GetTypeInfo() const { return _type; };
    asIScriptObject* GetRaw() const { return _object; };
    const std::string GetComponentPath();

    YAML::Node BuildYaml(const YAML::Node& origDesc) const;

private:
    void Init(asITypeInfo* type, ArgsT&& args = {}, asIScriptFunction* factory = nullptr);
    asIScriptObject* Construct(ArgsT&& args = {}, asIScriptFunction* factory = nullptr);
    asIScriptFunction* FindAppropriateFactory(const ArgsT& args);

    void OverrideObject(const std::string &fieldName, const YAML::Node &node);
    void OverrideNativeObject(const std::string &fieldName, const asUINT fieldType, const YAML::Node &node);
    void OverrideResource(const std::string &fieldName, const YAML::Node &node);
    void OverrideArray(const std::string &fieldName, const YAML::Node &node);
    void OverrideChildren(const YAML::Node &node);
    void OverrideRef(const std::string &fieldName, const std::string &ref);
    void OverrideSimpleField(const std::string &fieldName, const std::string &value);

    void BuildFieldYaml(std::string_view name, asUINT fieldIdx, YAML::Node& parent, const YAML::Node& origDesc) const;

    bool GetChildSceneRepr(std::string_view name, YAML::Node& out) const;

private:
    std::unordered_map<std::string, asUINT> _fields;
    asITypeInfo* _type = nullptr;
    asIScriptObject* _object = nullptr;
};