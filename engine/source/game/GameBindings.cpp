#include "GameBindings.h"

#include "scripts/ScriptSystem.h"
#include "shared/ResourceHandle.h"

#include "scene/SceneLoader.h"
#include "PrefabLoader.h"
#include "scripts/ComponentLoader.h" // TODO: move to game folder?
#include "logs/Logs.h"

#include <scripthandle.h>

using namespace GameBindings;

namespace GameBindings {

static asIScriptFunction* destroyComponentFunc = nullptr;

void SetDestroyComponentCB(asIScriptFunction* destroyCompCb)
{
    assert(destroyComponentFunc == nullptr);
    destroyComponentFunc = destroyCompCb;
}

void DestroyComponent(ScriptObject& comp)
{
    if (!destroyComponentFunc) {
        LOG_ERROR("failed to destroy component with native call GameBindings::DestroyComponent. destroy component callback wasn't ever set");
        return;
    }
    CScriptHandle compHandle(comp.GetRaw(), comp.GetTypeInfo());
    gScriptSys->GetEngine()->CallFunction(destroyComponentFunc, [&compHandle](asIScriptContext* context) {
        context->SetArgObject(0, &compHandle);
    });
}

} // GameBindings

CScriptHandle CreateScene(const CResourceHandle& sceneRes) 
{
    CScriptHandle ref;
    if (!sceneRes.IsValid()) {
        LOG_ERROR("failed to load scene. scene reference was invalid");
        return ref;
    }
    std::unique_ptr<ScriptObject> sceneInst = 
        SceneLoader::CreateScene(sceneRes.GetTag());
    if (sceneInst) {
        ref.Set(sceneInst->GetRaw(), sceneInst->GetTypeInfo());
    }
    return ref;
}

CScriptHandle SpawnPrefab(const CResourceHandle& prefabRes, CScriptHandle parent = CScriptHandle())
{
    CScriptHandle ref;
    if (!prefabRes.IsValid()) {
        LOG_ERROR("failed to spawn prefab. prefab reference was invalid");
        return ref;
    }
    ScriptObject* prefabInst = nullptr;
    if (parent.GetRef()) {
        ScriptObject parentObj(static_cast<asIScriptObject*>(parent.GetRef()));
        prefabInst = PrefabLoader::Create(prefabRes.GetTag(), &parentObj);
    } else {
        prefabInst = PrefabLoader::Create(prefabRes.GetTag());
    }
    if (prefabInst) {
        ref.Set(prefabInst->GetRaw(), prefabInst->GetTypeInfo());
        delete prefabInst;
    }
    return ref;
}

CScriptHandle SpawnComponent(const CResourceHandle& compRef, CScriptHandle parent = CScriptHandle())
{
    CScriptHandle ref;
    if (!compRef.IsValid()) {
        LOG_ERROR("failed to spawn component. component reference was invalid");
        return ref;
    }
    ScriptObject* compInst = nullptr;
    if (parent.GetRef()) {
        ScriptObject parentObj(static_cast<asIScriptObject*>(parent.GetRef()));
        compInst = ComponentLoader::CreateComponent(compRef.GetTag(), &parentObj);
    } else {
        compInst = ComponentLoader::CreateComponent(compRef.GetTag());
    }
    if (compInst) {
        ref.Set(compInst->GetRaw(), compInst->GetTypeInfo());
        delete compInst;
    }
    return ref;
}

void RegisterGameBindings()
{
    auto engine = gScriptSys->GetRawEngine();
    int r;
    
    engine->SetDefaultNamespace("Game::Impl");

    r = engine->RegisterGlobalFunction("ref@ CreateScene(const ResourceHandle &in scene)", asFUNCTION(CreateScene), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("ref@ SpawnPrefab(const ResourceHandle &in prefab, ref@ parent = null)", asFUNCTION(SpawnPrefab), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("ref@ SpawnComponent(const ResourceHandle &in compRef, ref@ parent = null)", asFUNCTION(SpawnComponent), asCALL_CDECL); assert(r >= 0);

    engine->SetDefaultNamespace("");
}
