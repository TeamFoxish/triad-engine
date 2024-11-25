#pragma once

#include "angelscript.h"
#include <memory>
#include <string>
#include "misc/Function.h"

class ScriptEngine {
private:
    asIScriptEngine* _engine;
public:
    ScriptEngine();
    ~ScriptEngine();
    asIScriptEngine* GetEngine() { return this->_engine; }
    asIScriptModule* GetModule(const std::string& module) { return this->_engine->GetModule(module.c_str()); }
    bool CallFunction(asIScriptFunction* function);
    bool CallFunction(asIScriptFunction* function, Consumer<asIScriptContext*>&& argsSetter);
    bool CallFunctionAndGet(asIScriptFunction* function, Consumer<asIScriptContext*>&& retValueGetter);
    bool CallFunctionAndGet(
        asIScriptFunction* function,
        Consumer<asIScriptContext*>&& argsSetter,
        Consumer<asIScriptContext*>&& retValueGetter);
};