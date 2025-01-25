#include "ScriptEngine.h"
#include "ScriptRegistry.h"
#include <iostream>
#include <format>
#include "misc/Function.h"
#include "logs/Logs.h"
#include "debugger.h"
#include "ScriptSystem.h"

asIScriptContext* CallbackContextRequest(asIScriptEngine* engine, void*)
{
    asIScriptContext* context = gScriptSys->GetContext();
    if (!context) {
        context = engine->CreateContext();
        LOG_INFO("Default script context initialized");
#ifdef EDITOR
        CDebugger* debugger = gScriptSys->GetDebugger();
        context->SetLineCallback(asMETHOD(CDebugger, LineCallback), debugger, asCALL_THISCALL);
        LOG_INFO("Debugger attached to default context");
#endif // EDITOR

    }
    return context;
}

void CallbackContextReturn(asIScriptEngine* engine, asIScriptContext* context, void*)
{
}

ScriptEngine::ScriptEngine()
{
    _engine = asCreateScriptEngine();
    int r = _engine->SetMessageCallback(asFUNCTION(ScriptRegistry::StdMessageCallback), 0, asCALL_CDECL);
    if (r < 0) {
        std::cout << "Unrecoverable error while setting message callback." << std::endl;
    }

    //r = _engine->SetContextCallbacks(CallbackContextRequest, CallbackContextReturn, nullptr);
    if ( r < 0) {
        LOG_ERROR("Failed to bind context callbacks");
    }
}

ScriptEngine::~ScriptEngine()
{
    _engine->ShutDownAndRelease();
}

bool ScriptEngine::CallFunction(asIScriptFunction *function)
{
    asIScriptContext* context = _engine->RequestContext();
    context->AddRef();

    context->Prepare(function);

    int r = context->Execute();

    if( r != asEXECUTION_FINISHED ) {
        if ( r == asEXECUTION_EXCEPTION ) {
             const asIScriptFunction *function = context->GetExceptionFunction();
            LOG_ERROR("Exception \"{}\". Line \"{}\" at:\n\tModule: \"{}\"\n\tFunction: \"{}\"\n\tFile: \"{}\"",
                context->GetExceptionString(),
                context->GetExceptionLineNumber(),
                function->GetModuleName(),
                function->GetDeclaration(),
                function->GetScriptSectionName());
            return false;
        }
    }
    context->Release();
    return true;
}

bool ScriptEngine::CallFunction(asIScriptFunction* function, Consumer<asIScriptContext*>&& argsSetter) {
    asIScriptContext* context = _engine->RequestContext();
    context->AddRef();

    context->Prepare(function);

    argsSetter(context);

    int r = context->Execute();

    if( r != asEXECUTION_FINISHED ) {
        if ( r == asEXECUTION_EXCEPTION ) {
            const asIScriptFunction *function = context->GetExceptionFunction();
            LOG_ERROR("Exception \"{}\". Line \"{}\" at:\n\tModule: \"{}\"\n\tFunction: \"{}\"\n\tFile: \"{}\"",
                context->GetExceptionString(),
                context->GetExceptionLineNumber(),
                function->GetModuleName(),
                function->GetDeclaration(),
                function->GetScriptSectionName());
            return false;
        }
    }
    context->Release();
    return true;
}

bool ScriptEngine::CallFunctionAndGet(asIScriptFunction *function, Consumer<asIScriptContext*>&& retValueGetter)
{
    asIScriptContext* context = _engine->RequestContext();
    context->AddRef();

    context->Prepare(function);

    int r = context->Execute();

    if( r != asEXECUTION_FINISHED ) {
        if ( r == asEXECUTION_EXCEPTION ) {
             const asIScriptFunction *function = context->GetExceptionFunction();
            LOG_ERROR("Exception \"{}\". Line \"{}\" at:\n\tModule: \"{}\"\n\tFunction: \"{}\"\n\tFile: \"{}\"",
                context->GetExceptionString(),
                context->GetExceptionLineNumber(),
                function->GetModuleName(),
                function->GetDeclaration(),
                function->GetScriptSectionName());
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
    asIScriptContext* context = _engine->RequestContext();
    context->AddRef();

    context->Prepare(function);

    argsSetter(context);

    int r = context->Execute();

    if( r != asEXECUTION_FINISHED ) {
        if ( r == asEXECUTION_EXCEPTION ) {
             const asIScriptFunction *function = context->GetExceptionFunction();
            LOG_ERROR("Exception \"{}\". Line \"{}\" at:\n\tModule: \"{}\"\n\tFunction: \"{}\"\n\tFile: \"{}\"",
                context->GetExceptionString(),
                context->GetExceptionLineNumber(),
                function->GetModuleName(),
                function->GetDeclaration(),
                function->GetScriptSectionName());
            return false;
        }
    }

    retValueGetter(context);

    context->Release();
    return true;
}
