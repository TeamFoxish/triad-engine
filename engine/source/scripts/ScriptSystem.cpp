#include "ScriptSystem.h"
#include <assert.h>
#include <scriptstdstring.h>
#include <scriptbuilder.h>
#include <iostream>
#include <format>
#include "scripts/ScriptRegistry.h"


std::unique_ptr<ScriptSystem> gScriptSys = nullptr;

bool InitScript(RuntimeIface *runtime)
{
    assert(gScriptSys == nullptr);
    gScriptSys = std::make_unique<ScriptSystem>();
    return gScriptSys->Init(runtime);
}

void TermScript(RuntimeIface *runtime)
{
    assert(gScriptSys);
    gScriptSys->Term();
    gScriptSys = nullptr;
}

bool ScriptSystem::Init(RuntimeIface *runtime)
{
    _engine = new ScriptEngine();
    _registry = new ScriptRegistry();
    if (!_registry->RegisterStdLibrary(_engine->GetEngine())) {
        std::cout << "Failed to register Std Script library." << std::endl;
        return false;
    }
    return true;
}

void ScriptSystem::Term()
{
}

bool ScriptSystem::CallFunction(const std::string &module, const std::string &signature)
{
    asIScriptFunction* function = _registry->GetFunction(module, signature);
    return _engine->CallFunction(function);
}

bool ScriptSystem::Update(float deltaTime) {
    asIScriptFunction* function = _registry->GetUpdateFunction();
    return _engine->CallFunction(function, [deltaTime] (asIScriptContext* context) {
        context->SetArgFloat(0, deltaTime);
    });

    // asIScriptFunction* function = _registry->GetUpdateFunction();
    // asIScriptContext* context = _engine->GetEngine()->CreateContext();

    // context->Prepare(function);

    // context->SetArgFloat(0, deltaTime);

    // int r = context->Execute();

    // if( r != asEXECUTION_FINISHED ) {
    //     if ( r == asEXECUTION_EXCEPTION ) {
    //         std::cout << std::format("An exception '%s' occurred. Please correct the code and try again.", context->GetExceptionString()) << std::endl;
    //         return false;
    //     }
    // }
    // context->Release();
    // return true;
}

bool ScriptSystem::FixedUpdate(float deltaTime) {
    asIScriptFunction* function = _registry->GetFixedUpdateFuction();
    return _engine->CallFunction(function, [deltaTime] (asIScriptContext* context) {
        context->SetArgFloat(0, deltaTime);
    });

    // asIScriptFunction* function = _registry->GetFixedUpdateFuction();
    // asIScriptContext* context = _engine->GetEngine()->CreateContext();

    // context->Prepare(function);

    // context->SetArgFloat(0, deltaTime);

    // int r = context->Execute();

    // if( r != asEXECUTION_FINISHED ) {
    //     if ( r == asEXECUTION_EXCEPTION ) {
    //         std::cout << std::format("An exception '%s' occurred. Please correct the code and try again.", context->GetExceptionString()) << std::endl;
    //         return false;
    //     }
    // }
    // context->Release();
    // return true;
}

bool ScriptSystem::PostInitEvent() {
    return CallFunction("Engine", "void init()");
}
