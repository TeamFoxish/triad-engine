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


asIScriptFunction* updateCallback = 0;
asIScriptFunction* fixedUpdateCallback = 0;
asIScriptFunction* shutdownCallback = 0;

ENGINE_API void Print(const std::string& msg) {
    std::cout << msg;
}

ENGINE_API void PrintLn(const std::string& msg) {
    std::cout << msg << std::endl;
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

std::string PropToString(const char* name, int type, void* value) {
    std::string val;
    if (type == 4) {
        auto* fl = static_cast<uint32_t*>(value);
        val = std::to_string(*fl);
        *fl = *fl + 1;
    }
    if (type == 10) {
        val = std::to_string(*static_cast<float*>(value));
    }
    auto* ti = gScriptSys->GetRawEngine()->GetTypeInfoById(type);
    if (ti != nullptr && ti ->GetName() != "string" && ti->GetName() != "array") {
        asIScriptObject* obj = *static_cast<asIScriptObject**>(value);
        asIScriptGeneric* j = reinterpret_cast<asIScriptGeneric*>(value);
        gScriptSys->GetEngine();

    }
    return  "Name: " + std::string(name) + " Type: " + (ti == nullptr ? std::to_string(type) : ti->GetName()) + " Value: " + val;
}

ENGINE_API asIScriptObject* LoadScene(Strid sceneName) {
    return nullptr;
}

ENGINE_API void SaveScene(asIScriptObject* sceneRoot) {
    
}

bool ScriptRegistry::RegisterCustomFunctions(asIScriptEngine *engine)
{
    int r;

    // Standard funtion 'print' to output text to console
    r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(Print), asCALL_CDECL);
    if (r < 0) {
        std::cout << "Unrecoverable error while binding 'print' function." << std::endl;
        return false;
    }

    // Standard funtion 'println' to output text to console and add new line
    r = engine->RegisterGlobalFunction("void println(const string &in)", asFUNCTION(PrintLn), asCALL_CDECL);
    if (r < 0) {
        std::cout << "Unrecoverable error while binding 'println' function." << std::endl;
        return false;
    }

    // Set Update callback
    r = engine->RegisterFuncdef("void Update(float)");
    if (r < 0) {
        std::cout << "Unrecoverable error while binding 'Update' callback." << std::endl;
        return false;
    }

    // Setter for OnUpdate callback
    r = engine->RegisterGlobalFunction("void SetUpdate(Update @updateCallback)", asFUNCTION(SetOnUpdate), asCALL_CDECL);
    if (r < 0) {
        std::cout << "Unrecoverable error while binding 'SetUpdate' update function setter." << std::endl;
        return false;
    }

    // Set FixedUpdate callback
    r = engine->RegisterFuncdef("void FixedUpdate(float)");
    if (r < 0) {
        std::cout << "Unrecoverable error while binding 'FixedUpdate' callback." << std::endl;
        return false;
    }

    // Setter for FixedUpdate callback
    r = engine->RegisterGlobalFunction("void SetFixedUpdate(FixedUpdate @updateCallback)", asFUNCTION(SetOnFixedUpdate), asCALL_CDECL);
    if (r < 0) {
        std::cout << "Unrecoverable error while binding 'SetFixedUpdate' fixed update function setter." << std::endl;
        return false;
    }

    // Set FixedUpdate callback
    r = engine->RegisterFuncdef("void Shutdown()");
    if (r < 0) {
        std::cout << "Unrecoverable error while binding 'Shutdown' callback funcdef." << std::endl;
        return false;
    }

    // Setter for Shutdown callback
    r = engine->RegisterGlobalFunction("void SetShutdown(Shutdown @shutdownCallback)", asFUNCTION(SetOnShutdown), asCALL_CDECL);
    if (r < 0) {
        std::cout << "Unrecoverable error while binding 'SetShutdown' callback." << std::endl;
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
