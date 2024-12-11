#include "ScriptSystem.h"
#include <assert.h>
#include <scriptstdstring.h>
#include <scriptbuilder.h>
#include <iostream>
#include <format>
#include "scripts/ScriptRegistry.h"
#include "scriptdictionary.h"

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
    _loader = static_cast<ScriptLoader*>(Factory<ResourceLoader>::Create("script").get());
    if (!_registry->RegisterStdLibrary(_engine->GetEngine())) {
        std::cout << "Failed to register Std Script library." << std::endl;
        return false;
    }
    _loader->LoadAll("./assets/scripts/");
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

bool ScriptSystem::CallFunction(const std::string &module, const std::string &signature, Consumer<asIScriptContext*>&& argsSetter) {
    asIScriptFunction* function = _registry->GetFunction(module, signature);
    return _engine->CallFunction(function, std::move(argsSetter));
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
    _loader->Build();
}

asIScriptObject *ScriptSystem::CreateComponentHandle(std::string* id)
{
    ScriptObject* handle = new ScriptObject("Engine", "ComponentHandle");
    handle->SetField("id", id);
    return handle->GetRaw();
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
