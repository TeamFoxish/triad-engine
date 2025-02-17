#include "SceneBindings.h"

#include "scripts/ScriptSystem.h"
#include "shared/MathScriptBindings.h"
#include "scene/SceneLoader.h"
#include "render/RenderSystem.h"
#include "logs/Logs.h"

#include <scripthandle.h>

static constexpr EntityId InvalidEntityId = {};

static std::unordered_map<const asIScriptObject*, SceneTree::Handle> objToEntity;

int32_t GetEntityInt32(asQWORD id)
{ 
    EntityId entId;
    entId.id = id;
    return entId.handle.id_;
}

static bool IsValidEntity(EntityId id) 
{
    return id.id != EntityId{}.id && gSceneTree->IsValidHandle(id.handle);
}

SceneTree::Handle GetEntityHandleFromScriptObject(const asIScriptObject* obj)
{
    const auto iter = objToEntity.find(obj);
    return iter != objToEntity.end() ? iter->second : SceneTree::Handle{};
}

asQWORD GetEntityIdFromHandle(SceneTree::Handle handle) 
{
    return EntityId(handle).id;
}

struct CEntityInfo {
    CScriptHandle entity;
    EntityId parent;
    // CScriptArray children; a pain in ass to construct
    std::string_view name;
    bool isComposite = false;

    std::string GetName() const { return !name.empty() ? std::string(name) : ""; }
    void SetName(const std::string& _name) { name = _name; }

    static void Create(CEntityInfo* self) { new(self) CEntityInfo(); }
    static void Destroy(CEntityInfo* self) { self->~CEntityInfo(); }
};

static EntityId AddEntityToSceneTree(CEntityInfo& info)
{
    assert((info.entity.GetTypeId() & asTYPEID_SCRIPTOBJECT) > 0 && (info.entity.GetTypeId() & asTYPEID_OBJHANDLE) > 0);
    
    ScriptObject obj(static_cast<asIScriptObject*>(info.entity.GetRef()));
    SceneTree::Entity entity(obj);
    entity.parent = info.parent.handle;
    entity.name = info.name;
    entity.isComposite = info.isComposite;
    if (!entity.isComposite && IsValidEntity(info.parent)) {
        entity.transform = gSceneTree->Get(entity.parent).transform; // useless??
    }
    // composite should set its transform individually by calling AddEntityTransformToSceneTree

    EntityId id;
    id.handle = gSceneTree->Add(std::move(entity));
    objToEntity[obj.GetRaw()] = id.handle;
    return id;
}

static void AddEntityTransformToSceneTree(EntityId id, const CTransformHandle* transform)
{
    SceneTree::Entity& entity = gSceneTree->Get(id.handle);
    entity.transform = transform->GetHandle();
}

static void RemoveEntityFromSceneTree(EntityId id) 
{
    const SceneTree::Entity& entity = gSceneTree->Get(id.handle);
    if (SceneLoader::FindSpawnedComponent(entity.obj)) {
        LOG_ERROR("component should be deleted before its entity is getting destroyed");
        SceneLoader::RemoveSpawnedComponent(id.handle);
    }
    objToEntity.erase(entity.obj.GetRaw());
    gSceneTree->Remove(id.handle);
}

CScriptHandle GetEntityById(EntityId id)
{
    CScriptHandle ref;
    if (!IsValidEntity(id)) {
        LOG_ERROR("failed to GetEntityById. no entity with id {} was found", id.id);
        return ref;
    }
    const SceneTree::Entity& ent = gSceneTree->Get(id.handle);
    if (!ent.obj.GetRaw() || !ent.obj.GetTypeInfo()) {
        LOG_ERROR("failed to GetEntityById {}. corresponding script object was invalid", id.id);
        return ref;
    }
    ref.Set(ent.obj.GetRaw(), ent.obj.GetTypeInfo());
    return ref;
}

static EntityId GetEntityIdUnderCursor() 
{
    const int32_t id = gRenderSys->GetEntityIdUnderCursor();
    if (id < 0) {
        return InvalidEntityId;
    }
    EntityId resId;
    resId.id = id;
    if (!gSceneTree->IsValidHandle(resId.handle)) {
        return InvalidEntityId;
    }
    return resId;
}

void RegisterSceneBindings()
{
    auto engine = gScriptSys->GetRawEngine();
    int r;

    engine->SetDefaultNamespace("Scene");

    r = engine->RegisterTypedef("EntityId", "uint64"); assert(r >= 0);
    r = engine->RegisterGlobalProperty("const EntityId EntityInvalidId", const_cast<EntityId*>(&InvalidEntityId)); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool IsValidEntity(EntityId id)", asFUNCTION(IsValidEntity), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("EntityId GetEntityIdUnderCursor()", asFUNCTION(GetEntityIdUnderCursor), asCALL_CDECL); assert(r >= 0);

    // Register the type
    r = engine->RegisterObjectType("Entity", sizeof(CEntityInfo), asOBJ_VALUE | asGetTypeTraits<CEntityInfo>()); assert(r >= 0);

    r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CEntityInfo::Create), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CEntityInfo::Destroy), asCALL_CDECL_OBJLAST); assert(r >= 0);

    // Register the object properties
    r = engine->RegisterObjectProperty("Entity", "ref@ entity", asOFFSET(CEntityInfo, entity)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Entity", "EntityId parent", asOFFSET(CEntityInfo, parent)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Entity", "bool isComposite", asOFFSET(CEntityInfo, isComposite)); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "const string& get_name() property", asMETHOD(CEntityInfo, GetName), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "void set_name(string &in) property", asMETHOD(CEntityInfo, SetName), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "Entity &opAssign(Entity &in)", asMETHODPR(CEntityInfo, operator=, (const CEntityInfo&), CEntityInfo&), asCALL_THISCALL); assert(r >= 0);

    engine->SetDefaultNamespace("Scene::Tree");

    engine->RegisterGlobalFunction("EntityId AddEntity(Entity &in entity)", asFUNCTION(AddEntityToSceneTree), asCALL_CDECL);
    engine->RegisterGlobalFunction("void AddEntityTransform(EntityId id, const Math::Transform@+ transform)", asFUNCTION(AddEntityTransformToSceneTree), asCALL_CDECL);
    engine->RegisterGlobalFunction("void RemoveEntity(EntityId id)", asFUNCTION(RemoveEntityFromSceneTree), asCALL_CDECL);
    engine->RegisterGlobalFunction("ref@ GetEntityById(EntityId id)", asFUNCTION(GetEntityById), asCALL_CDECL);

    engine->SetDefaultNamespace("");
}
