#include "ScriptSystem.h"
#include <assert.h>
#include <scriptstdstring.h>
#include <scriptbuilder.h>
#include <iostream>
#include <format>
#include "scripts/ScriptRegistry.h"
#include "scriptdictionary.h"

#include "logs/Logs.h"

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
    _context = GetRawEngine()->CreateContext();
    _registry = new ScriptRegistry();
    if (!_registry->RegisterStdLibrary(_engine->GetEngine())) {
        std::cout << "Failed to register Std Script library." << std::endl;
        return false;
    }
    _loader.LoadAll("./assets/scripts/");
    return true;
}

void ScriptSystem::Term()
{
    asIScriptFunction* shutdownFunc = _registry->GetShutdownFuction();
    const bool success = _engine->CallFunction(shutdownFunc);
    if (!success) {
        LOG_ERROR("failed to terminate script system. script Shutdown function returned false");
        return;
    }
    delete _registry;
    _registry = nullptr;
    _context->Release();
    _context = nullptr;
    delete _engine;
    _engine = nullptr;
}

bool ScriptSystem::CallFunction(const std::string &module, const std::string &signature)
{
    asIScriptFunction* function = _registry->GetFunction(module, signature);
    return _engine->CallFunction(function);
}

bool ScriptSystem::CallFunction(const std::string &module, const std::string &signature, Consumer<asIScriptContext*>&& argsSetter) {
    asIScriptFunction* function = _registry->GetFunction(module, signature);
    return _engine->CallFunction(function, std::move(argsSetter));
}

bool ScriptSystem::CallFunctionAndGet(const std::string &module, const std::string &signature, Consumer<asIScriptContext*>&& retValueGetter) {
    asIScriptFunction* function = _registry->GetFunction(module, signature);
    return _engine->CallFunctionAndGet(function, std::move(retValueGetter));
}


bool ScriptSystem::Update(float deltaTime) {
    asIScriptFunction* function = _registry->GetUpdateFunction();
    return _engine->CallFunction(function, [deltaTime] (asIScriptContext* context) {
        context->SetArgFloat(0, deltaTime);
    });
}

bool ScriptSystem::FixedUpdate(float deltaTime) {
    asIScriptFunction* function = _registry->GetFixedUpdateFuction();
    return _engine->CallFunction(function, [deltaTime] (asIScriptContext* context) {
        context->SetArgFloat(0, deltaTime);
    });
}

bool ScriptSystem::PostInitEvent() {
    return CallFunction("Engine", "void init()");
}

asITypeInfo *ScriptSystem::GetArrayType()
{
    if (_arrayType == nullptr) {
        _arrayType = GetRawEngine()->GetTypeInfoByDecl("array");
    }
    return _arrayType;
}

asITypeInfo *ScriptSystem::GetStringType()
{
    if (_stringType == nullptr) {
        _stringType = GetRawEngine()->GetTypeInfoByDecl("string");
    }
    return _stringType;
}

asITypeInfo *ScriptSystem::GetDictionaryType()
{
    if (_dictType == nullptr) {
        _dictType = GetRawEngine()->GetTypeInfoByDecl("dictionary");
    }
    return _dictType;
}

void ScriptSystem::BuildModules()
{
    _loader.Build();
}

asIScriptObject *ScriptSystem::CreateComponentHandle(std::string* id)
{
    ScriptObject* handle = new ScriptObject("Engine", "ComponentHandle");
    handle->SetField("id", id);
    asIScriptObject* raw = handle->GetRaw();
    delete handle;
    return raw;
}

void ScriptSystem::AddComponentToContext(ScriptObject *component, uint64_t id)
{
    CallFunction("Engine", "void AddToStorage(BaseComponent@ component, uint64 id)", [component, id] (asIScriptContext* context) {
        context->SetArgObject(0, component);
        context->SetArgQWord(1, id);
    });
}

void ScriptSystem::SetScene(ScriptObject *sceneRoot)
{
    CallFunction("Engine", "void SetScene(Scene@ scene)", [sceneRoot] (asIScriptContext* context) {
        asIScriptObject* valObj = sceneRoot->GetRaw();
        context->SetArgObject(0, valObj);
    });
}

void ScriptSystem::LogCallstack()
{
    if (!_context) {
        return;
    }
    LOG_DEBUG("Script callstack:");
    for (asUINT n = 0; n < _context->GetCallstackSize(); n++)
    {
        asIScriptFunction* func;
        const char* scriptSection;
        int line, column;
        func = _context->GetFunction(n);
        line = _context->GetLineNumber(n, &column, &scriptSection);
        LOG_DEBUG("{}:{}:{},{}\n", scriptSection,
            func->GetDeclaration(),
            line, column);
    }
}
