#include "ScriptEngine.h"
#include "ScriptRegistry.h"
#include <iostream>
#include <format>
#include "misc/Function.h"

ScriptEngine::ScriptEngine()
{
    _engine = asCreateScriptEngine();
    int r = _engine->SetMessageCallback(asFUNCTION(ScriptRegistry::StdMessageCallback), 0, asCALL_CDECL);
    if (r < 0) {
        std::cout << "Unrecoverable error while setting message callback." << std::endl;
    }
}

ScriptEngine::~ScriptEngine()
{
    _engine->Release();
}

bool ScriptEngine::CallFunction(asIScriptFunction *function)
{
    asIScriptContext* context = _engine->CreateContext();

    context->Prepare(function);

    int r = context->Execute();

    if( r != asEXECUTION_FINISHED ) {
        if ( r == asEXECUTION_EXCEPTION ) {
            std::cout << std::format("An exception '%s' occurred. Please correct the code and try again.", context->GetExceptionString()) << std::endl;
            return false;
        }
    }
    context->Release();
    return true;
}

bool ScriptEngine::CallFunction(asIScriptFunction* function, Consumer<asIScriptContext*>&& argsSetter) {
    asIScriptContext* context = _engine->CreateContext();

    context->Prepare(function);

    argsSetter(context);

    int r = context->Execute();

    if( r != asEXECUTION_FINISHED ) {
        if ( r == asEXECUTION_EXCEPTION ) {
            std::cout << std::format("An exception '%s' occurred. Please correct the code and try again.", context->GetExceptionString()) << std::endl;
            return false;
        }
    }
    context->Release();
    return true;
}

bool ScriptEngine::CallFunctionAndGet(asIScriptFunction *function, Consumer<asIScriptContext*>&& retValueGetter)
{
    asIScriptContext* context = _engine->CreateContext();

    context->Prepare(function);

    int r = context->Execute();

    if( r != asEXECUTION_FINISHED ) {
        if ( r == asEXECUTION_EXCEPTION ) {
            std::cout << std::format("An exception '%s' occurred. Please correct the code and try again.", context->GetExceptionString()) << std::endl;
            return false;
        }
    }

    retValueGetter(context);

    context->Release();
    return true;
}

bool ScriptEngine::CallFunctionAndGet(asIScriptFunction *function,
 Consumer<asIScriptContext*>&& argsSetter,
 Consumer<asIScriptContext *>&& retValueGetter)
{
    asIScriptContext* context = _engine->CreateContext();

    context->Prepare(function);

    argsSetter(context);

    int r = context->Execute();

    if( r != asEXECUTION_FINISHED ) {
        if ( r == asEXECUTION_EXCEPTION ) {
            std::cout << std::format("An exception '%s' occurred. Please correct the code and try again.", context->GetExceptionString()) << std::endl;
            return false;
        }
    }

    retValueGetter(context);

    context->Release();
    return true;
}
