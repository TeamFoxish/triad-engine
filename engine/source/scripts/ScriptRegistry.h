#pragma once

#include "angelscript.h"
#include <string>
#include <unordered_map>

class ScriptRegistry {
private:
    // Caching previously requested functions. Module -> FunctionSignature -> FunctionPointer
    // static inline std::unordered_map<const std::string, std::unordered_map<const std::string, asIScriptFunction*>> _functionCache;
    static bool RegisterCustomFunctions(asIScriptEngine* engine);
public:
    // Registers all add-on's of Angel Script and then passing to our std functions
    static bool RegisterStdLibrary(asIScriptEngine* engine);
    // Message Callback function for Angel Scrip engine. Engine use this for passing logs from script engine to console.
    static void StdMessageCallback(const asSMessageInfo *msg, void *param);
    static asIScriptFunction* GetFunction(const std::string& moduleName, const std::string& signature);
    static asIScriptFunction* GetUpdateFunction();
    static asIScriptFunction* GetFixedUpdateFuction();
    static asIScriptFunction* GetShutdownFuction();
};