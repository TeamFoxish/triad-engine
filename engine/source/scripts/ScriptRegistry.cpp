#include "ScriptRegistry.h"
#include "ScriptSystem.h"
#include "ScriptEngine.h"
#include "angelscript.h"
#include "scriptstdstring.h"
#include "datetime.h"
#include "scriptarray.h"
#include "scriptany.h"
#include "scripthandle.h"
#include "weakref.h"
#include "scriptdictionary.h"
#include "scriptfile.h"
#include "scriptfilesystem.h"
#include "scriptmath.h"
#include "scriptmathcomplex.h"
#include "scriptgrid.h"
#include "scripthelper.h"
#include "misc/API.h"
#include <iostream>
#include <string>
#include "misc/Strid.h"
#include "game/GameBindings.h"
#include "logs/Logs.h"


asIScriptFunction* updateCallback = 0;
asIScriptFunction* fixedUpdateCallback = 0;
asIScriptFunction* shutdownCallback = 0;

ENGINE_API void Print(const std::string& msg) {
    std::cout << msg;
}

ENGINE_API void PrintLn(const std::string& msg) {
    std::cout << msg << std::endl;
}

ENGINE_API void log_info(const std::string& msg) {
    LOG_INFO(msg);
}

ENGINE_API void log_error(const std::string& msg) {
    LOG_ERROR(msg);
}

ENGINE_API void log_warn(const std::string& msg) {
    LOG_WARN(msg);
}

ENGINE_API void log_critical(const std::string& msg) {
    LOG_CRIT(msg);
}

ENGINE_API void log_debug(const std::string& msg) {
    LOG_DEBUG(msg);
}

ENGINE_API void SetOnUpdate(asIScriptFunction* updateFunc) {
    if ( updateCallback ) {
        updateCallback->Release();
    }
    updateCallback = updateFunc;
}

ENGINE_API void SetOnFixedUpdate(asIScriptFunction* fixedUpdateFunc) {
    if ( fixedUpdateCallback ) {
        fixedUpdateCallback->Release();
    }
    fixedUpdateCallback = fixedUpdateFunc;
}

ENGINE_API void SetOnShutdown(asIScriptFunction* shutdownFunc) {
    if (shutdownCallback) {
        shutdownCallback->Release();
    }
    shutdownCallback = shutdownFunc;
}

ENGINE_API asIScriptObject* LoadScene(Strid sceneName) {
    return nullptr;
}

ENGINE_API void SaveScene(asIScriptObject* sceneRoot) {
    
}

ENGINE_API CScriptAny* CreateObject(const std::string& module, const std::string& classDeclaration) {
    asITypeInfo* type = gScriptSys
                            ->GetRawEngine()
                            ->GetModule(module.c_str())
                            ->GetTypeInfoByDecl(classDeclaration.c_str());
    asIScriptObject* object =  static_cast<asIScriptObject*>(gScriptSys
                            ->GetRawEngine()
                            ->CreateScriptObject(type));
    CScriptAny* res = new CScriptAny(object, type->GetTypeId(), gScriptSys->GetRawEngine());
    return res;
}

bool ScriptRegistry::RegisterCustomFunctions(asIScriptEngine *engine)
{
    int r;

    // Standard funtion 'print' to output text to console
    r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(Print), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'print' function.");
        return false;
    }

    // Standard funtion 'println' to output text to console and add new line
    r = engine->RegisterGlobalFunction("void println(const string &in)", asFUNCTION(PrintLn), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'println' function.");
        return false;
    }

    // Standard funtion 'log_critical' to log critical error
    r = engine->RegisterGlobalFunction("void log_critical(const string &in)", asFUNCTION(log_critical), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'log_critical' function.");
        return false;
    }

    // Standard funtion 'log_error' to log error
    r = engine->RegisterGlobalFunction("void log_error(const string &in)", asFUNCTION(log_error), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'log_error' function.");
        return false;
    }

    // Standard funtion 'log_warn' to log warn
    r = engine->RegisterGlobalFunction("void log_warn(const string &in)", asFUNCTION(log_warn), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'log_warn' function.");
        return false;
    }

    // Standard funtion 'log_info' to log info
    r = engine->RegisterGlobalFunction("void log_info(const string &in)", asFUNCTION(log_info), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'log_info' function.");
        return false;
    }

    // Standard funtion 'log_debug' to log info
    r = engine->RegisterGlobalFunction("void log_debug(const string &in)", asFUNCTION(log_debug), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'log_debug' function.");
        return false;
    }

    // Set Update callback
    r = engine->RegisterFuncdef("void Update(float)");
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'Update' callback.");
        return false;
    }

    // Setter for OnUpdate callback
    r = engine->RegisterGlobalFunction("void SetUpdate(Update @updateCallback)", asFUNCTION(SetOnUpdate), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'SetUpdate' update function setter.");
        return false;
    }

    // Set FixedUpdate callback
    r = engine->RegisterFuncdef("void FixedUpdate(float)");
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'FixedUpdate' callback.");
        return false;
    }

    // Setter for FixedUpdate callback
    r = engine->RegisterGlobalFunction("void SetFixedUpdate(FixedUpdate @updateCallback)", asFUNCTION(SetOnFixedUpdate), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'SetFixedUpdate' fixed update function setter.");
        return false;
    }

    // Shutdown
    r = engine->RegisterFuncdef("void Shutdown()");
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'Shutdown' callback funcdef.");
        return false;
    }
    r = engine->RegisterGlobalFunction("void SetShutdown(Shutdown @shutdownCallback)", asFUNCTION(SetOnShutdown), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'SetShutdown' callback.");
        return false;
    }

    // DestroyComponent
    r = engine->RegisterFuncdef("void CallbackDestroyComponent(ref@ component)");
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'CallbackDestroyComponent' callback funcdef.");
        return false;
    }
    r = engine->RegisterGlobalFunction("void SetDestroyComponent(CallbackDestroyComponent @destroyCompCallback)", asFUNCTION(GameBindings::SetDestroyComponentCB), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'SetDestroyComponent' callback.");
        return false;
    }
    r = engine->RegisterGlobalFunction("any@ CreateObject(const string &in module, const string &in classDeclaration)", asFUNCTION(CreateObject), asCALL_CDECL);
    if (r < 0) {
        LOG_ERROR("Unrecoverable error while binding 'CreateObject' callback.");
        return false;
    }

    return true;
}

bool ScriptRegistry::RegisterStdLibrary(asIScriptEngine *engine)
{
    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterStdStringUtils(engine);
    RegisterScriptDateTime(engine);
    RegisterScriptAny(engine);
    RegisterScriptHandle(engine);
    RegisterScriptWeakRef(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptFile(engine);
    RegisterScriptFileSystem(engine);
    engine->SetDefaultNamespace("Math");
    RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);
    RegisterScriptGrid(engine);
    engine->SetDefaultNamespace("");
    RegisterExceptionRoutines(engine);

    extern bool MathScriptBindingsInit();
    MathScriptBindingsInit();

    extern void RegisterSceneBindings();
    RegisterSceneBindings();

    extern void RegisterResourceHandles();
    RegisterResourceHandles();

    extern void RegisterLightsBindings();
    RegisterLightsBindings();

    extern void RegisterCameraBindings();
    RegisterCameraBindings();

    extern void RegisterInputBindings();
    RegisterInputBindings();

    extern void RegisterSoundBindings();
    RegisterSoundBindings();

    extern void RegisterPhysicsBindings();
    RegisterPhysicsBindings();

    extern void RegisterGameBindings();
    RegisterGameBindings();

    return RegisterCustomFunctions(engine);
}

void ScriptRegistry::StdMessageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING ) 
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION ) 
        type = "INFO";
    printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

asIScriptFunction* ScriptRegistry::GetFunction(const std::string& moduleName, const std::string& signature)
{
    // auto moduleMapIt = _functionCache.find(moduleName);
    // if (moduleMapIt != _functionCache.end()) {
    //     auto functionIt = moduleMapIt->second.find(signature);
    //     if (functionIt != moduleMapIt->second.end()) {
    //         return functionIt->second;
    //     }
    // }

    asIScriptModule* module = gScriptSys->GetEngine()->GetModule(moduleName);
    asIScriptFunction* function = module->GetFunctionByDecl(signature.c_str());
    //_functionCache[moduleName][signature] = function;
    return function;
}

asIScriptFunction *ScriptRegistry::GetUpdateFunction()
{
    return updateCallback;
}

asIScriptFunction *ScriptRegistry::GetFixedUpdateFuction()
{
    return fixedUpdateCallback;
}

asIScriptFunction* ScriptRegistry::GetShutdownFuction()
{
    return shutdownCallback;
}
