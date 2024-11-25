#pragma once

#include "angelscript.h"
#include <memory>
#include "runtime/RuntimeIface.h"
#include "ScriptLoader.h"
#include "ScriptEngine.h"
#include "ScriptRegistry.h"

class ScriptLoader;
class ScriptEngine;
class ScriptRegistry;

class ScriptSystem {
private:
    ScriptRegistry* _registry;
    ScriptLoader* _loader;
    ScriptEngine* _engine;
public:
    bool Init(RuntimeIface* runtime);
    void Term();

    ScriptEngine* GetEngine() { return this->_engine; }
    asIScriptEngine* GetRawEngine() { return this->_engine->GetEngine(); }
    ScriptLoader* GetScriptLoader() { return this->_loader; }
    bool CallFunction(const std::string& module, const std::string& signature);
    bool Update(float deltaTime);
    bool FixedUpdate(float deltaTime);
    bool PostInitEvent();
};

bool InitScript(RuntimeIface* runtime);
void TermScript(RuntimeIface* runtime);

extern std::unique_ptr<ScriptSystem> gScriptSys;

