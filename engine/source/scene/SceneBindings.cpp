#include "SceneBindings.h"

#include "scripts/ScriptSystem.h"
#include "scene/SceneTree.h"
#include "shared/MathScriptBindings.h"

#include <scripthandle.h>

union EntityId {
    SceneTree::Handle handle = {};
    asQWORD id;
};

static constexpr EntityId InvalidEntityId = {};

static bool IsValidEntity(EntityId id) 
{
    return id.id != EntityId{}.id;
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
    
    SceneTree::Entity entity(static_cast<asIScriptObject*>(info.entity.GetRef()));
    // TODO: assign info.entity
    entity.parent = info.parent.handle;
    entity.name = info.name;
    entity.isComposite = info.isComposite;
    if (!entity.isComposite && IsValidEntity(info.parent)) {
        entity.transform = gSceneTree->Get(entity.parent).transform; // useless??
    }
    // composite should set its transform individually by calling AddEntityTransformToSceneTree

    EntityId id;
    id.handle = gSceneTree->Add(std::move(entity));
    return id;
}

static void AddEntityTransformToSceneTree(EntityId id, const CTransformHandle* transform)
{
    SceneTree::Entity& entity = gSceneTree->Get(id.handle);
    entity.transform = transform->GetHandle();
}

static void RemoveEntityFromSceneTree(EntityId id) 
{
    gSceneTree->Remove(id.handle);
}

void RegisterSceneBindings()
{
    auto engine = gScriptSys->GetRawEngine();
    int r;

    engine->SetDefaultNamespace("Scene");

    r = engine->RegisterTypedef("EntityId", "uint64"); assert(r >= 0);
    r = engine->RegisterGlobalProperty("const EntityId EntityInvalidId", const_cast<EntityId*>(&InvalidEntityId)); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool IsValidEntity(EntityId id)", asFUNCTION(IsValidEntity), asCALL_CDECL); assert(r >= 0);

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

    engine->SetDefaultNamespace("");
}
