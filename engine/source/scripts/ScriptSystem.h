#pragma once

#include "angelscript.h"
#include <memory>
#include "runtime/RuntimeIface.h"
#include "ScriptLoader.h"
#include "ScriptEngine.h"
#include "ScriptRegistry.h"
#include "ScriptObject.h"

class ScriptSystem {
private:
    ScriptRegistry* _registry = nullptr;
    ScriptLoader _loader;
    ScriptEngine* _engine = nullptr;
    asIScriptContext* _context = nullptr;
    asITypeInfo* _stringType = nullptr;
    asITypeInfo* _arrayType = nullptr;
    asITypeInfo* _dictType = nullptr;
public:
    bool Init(RuntimeIface* runtime);
    void Term();

    ScriptEngine* GetEngine() { return this->_engine; }
    asIScriptEngine* GetRawEngine() { return this->_engine->GetEngine(); }
    asIScriptContext* GetContext() const { return _context; }
    const ScriptLoader& GetScriptLoader() const { return _loader; }
    bool CallFunction(const std::string& module, const std::string& signature);
    bool CallFunction(const std::string &module, const std::string &signature, Consumer<asIScriptContext *> &&argsSetter);
    bool Update(float deltaTime);
    bool FixedUpdate(float deltaTime);
    bool PostInitEvent();
    void InitComponent(asIScriptObject* obj);
    asITypeInfo* GetArrayType();
    asITypeInfo* GetStringType();
    asITypeInfo *GetDictionaryType();
    void BuildModules();
    asIScriptObject* CreateComponentHandle(std::string* id);
    void AddComponentToContext(ScriptObject* component, uint64_t id);
    void SetScene(ScriptObject* sceneRoot);

    void LogCallstack();
};

bool InitScript(RuntimeIface* runtime);
void TermScript(RuntimeIface* runtime);

extern std::unique_ptr<ScriptSystem> gScriptSys;

