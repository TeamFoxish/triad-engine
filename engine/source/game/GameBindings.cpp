#include "GameBindings.h"

#include "scripts/ScriptSystem.h"
#include "shared/ResourceHandle.h"

#include "scene/SceneLoader.h"
#include "scene/SceneBindings.h"
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

void DestroyComponent(const ScriptObject& comp)
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
    if (compInst && compInst->GetRaw() && compInst->GetTypeInfo()) {
        ref.Set(compInst->GetRaw(), compInst->GetTypeInfo());
        delete compInst;
    }
    return ref;
}

struct ComponentGetter {
    CScriptHandle GetChildComponent(EntityId compId) const
    {
        CScriptHandle ref;
        if (!gSceneTree->IsValidHandle(compId.handle)) {
            LOG_ERROR("failed to get child component. component with id {} was invalid", compId.id);
            return ref;
        }
        const SceneTree::Entity& entity = gSceneTree->Get(compId.handle);
        if (!entity.isComposite) {
            LOG_ERROR("failed to get child component. component with id {} wasn't composite", compId.id);
            return ref;
        }
        if (!objType) {
            LOG_ERROR("failed to get child component. no component type passed as template or invalid", compId.id);
            return ref;
        }
        assert(objType->GetSubTypeCount() > 0);
        if (objType->GetSubTypeCount() == 0) {
            return ref;
        }
        const int subTypeId = objType->GetSubTypeId();
        if (subTypeId < 0) {
            return ref;
        }
        for (SceneTree::Handle childHandle : entity.children) {
            if (!gSceneTree->IsValidHandle(childHandle)) {
                return ref;
            }
            const SceneTree::Entity& child = gSceneTree->Get(childHandle);
            if (!child.obj.GetRaw() || !child.obj.GetTypeInfo()) {
                return ref;
            }
            if ((child.obj.GetTypeInfo()->GetTypeId() & subTypeId) != subTypeId) {
                continue; // wrong type
            }
            ref.Set(child.obj.GetRaw(), child.obj.GetTypeInfo());
            return ref;
        }
        return ref;
    }

    asITypeInfo* objType = nullptr;

    static void CreateDefault(asITypeInfo* objType, ComponentGetter* self) { new(self) ComponentGetter{objType}; }
    static void Destroy(ComponentGetter* self) { self->~ComponentGetter(); }
};

void RegisterGameBindings()
{
    auto engine = gScriptSys->GetRawEngine();
    int r;
    
    engine->SetDefaultNamespace("Game::Impl");

    r = engine->RegisterGlobalFunction("ref@ CreateScene(const ResourceHandle &in scene)", asFUNCTION(CreateScene), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("ref@ SpawnPrefab(const ResourceHandle &in prefab, ref@ parent = null)", asFUNCTION(SpawnPrefab), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("ref@ SpawnComponent(const ResourceHandle &in compRef, ref@ parent = null)", asFUNCTION(SpawnComponent), asCALL_CDECL); assert(r >= 0);

    engine->SetDefaultNamespace("Game::Query");

    r = engine->RegisterObjectType("ComponentGetter<class T>", sizeof(ComponentGetter), asOBJ_VALUE | asOBJ_TEMPLATE | asGetTypeTraits<ComponentGetter>()); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("ComponentGetter<T>", asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTION(ComponentGetter::CreateDefault), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("ComponentGetter<T>", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(ComponentGetter::Destroy), asCALL_CDECL_OBJLAST); assert(r >= 0);
	//r = engine->RegisterObjectMethod("ComponentGetter<T>", "ComponentGetter<T> &opAssign(const ComponentGetter<T> &in)", asMETHODPR(CResourceHandleDynamic, operator=, (const CResourceHandleDynamic&), CResourceHandleDynamic&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("ComponentGetter<T>", "ref@ GetChildComponent(Scene::EntityId id) const", asMETHOD(ComponentGetter, GetChildComponent), asCALL_THISCALL); assert(r >= 0);

    engine->SetDefaultNamespace("");
}
