#include "PhysicsBindings.h"

#include "scripts/ScriptSystem.h"
#include "physics/PhySystem.h"
#include "physics/Physics.h"
#include "scene/SceneBindings.h"
#include "shared/SharedStorage.h"
#include "shared/Shared.h"

#include <Jolt/Physics/Collision/Shape/SphereShape.h>

template<typename ChildT, typename ParamsT>
struct CShape {
    static void CreateDefault(ChildT* self) { new(self) ChildT(); }
    static void CreateCopy(ChildT* self, const ChildT& other) { new(self) ChildT(other); }
    static void Destroy(ChildT* self) { self->~ChildT(); }

    PhySystem::PhysicsHandle bodyHandle;
    JPH::Ref<ParamsT> params = static_cast<ChildT*>(this)->CreateParams();
};

struct CShapeSphere : public CNativeObject, public CShape<CShapeSphere, JPH::SphereShapeSettings> {
    static JPH::SphereShapeSettings* CreateParams() 
    {
        return new JPH::SphereShapeSettings(1.0f);
    }
    
    void ApplyOverrides(const YAML::Node& overrides) override
    {
        if (overrides["radius"]) {
            params->mRadius = overrides["radius"].as<float>();
        }
    }

    YAML::Node Serialize() const override 
    {
        YAML::Node node;
        node["radius"] = params->mRadius;
        return node;
    }
};

class CPhysBody {
public:
    CPhysBody() = default;

    CPhysBody(const CPhysBody& other) = delete;

    ~CPhysBody()
    {
        if (bodyHandle.id_ >= 0) {
            gPhySys->RemoveBody(bodyHandle);
        }
    }

    CPhysBody& operator=(CPhysBody&& other)
    {
        this->~CPhysBody();
        bodyHandle = PhySystem::PhysicsHandle{};
        std::swap(bodyHandle, other.bodyHandle);
        return *this;
    }

    void CreateBody(SceneTree::Handle entityHandle, const JPH::ShapeSettings& shapeParams)
    {
        const SceneTree::Entity& entity = gSceneTree->Get(entityHandle);
        const Math::Transform& trs = SharedStorage::Instance().transforms.AccessRead(entity.transform);
        const Math::Vector3 pos = trs.GetPosition();
        const Math::Quaternion rot = trs.GetRotation();
        const Math::Vector3 scale = trs.GetScale();
        JPH::Shape::ShapeResult shape = shapeParams.Create();
        const JPH::Vec3 scaleValid = shape.Get()->MakeScaleValid(JPH::Vec3(scale.x, scale.y, scale.z));
        shape = shape.Get()->ScaleShape(scaleValid);
        JPH::BodyCreationSettings settings(shape.Get(), JPH::RVec3(pos.x, pos.y, pos.z), JPH::Quat(rot.x, rot.y, rot.z, rot.w), JPH::EMotionType::Dynamic, Layers::MOVING);
        bodyHandle = gPhySys->AddBody(entityHandle, settings);
    }

    template<typename ShapeT>
    static void InitFromShape(CPhysBody* self, EntityId id, const ShapeT& shape) { self->CreateBody(id.handle, *shape.params); }

    static void CreateDefault(CPhysBody* self) { new(self) CPhysBody(); }
    static void Destroy(CPhysBody* self) { self->~CPhysBody(); }

protected:
    PhySystem::PhysicsHandle bodyHandle;
};

void RegisterPhysicsBindings()
{
    auto engine = gScriptSys->GetRawEngine();
    int r;

    r = engine->SetDefaultNamespace("Physics"); assert(r >= 0);

    r = engine->RegisterObjectType("ShapeSphere", sizeof(CShapeSphere), asOBJ_VALUE | asGetTypeTraits<CShapeSphere>()); assert(r >= 0);
    CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
    r = engine->RegisterObjectBehaviour("ShapeSphere", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CShapeSphere::CreateDefault), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("ShapeSphere", asBEHAVE_CONSTRUCT, "void f(const ShapeSphere &in)", asFUNCTION(CShapeSphere::CreateCopy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("ShapeSphere", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CShapeSphere::Destroy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectMethod("ShapeSphere", "ShapeSphere &opAssign(const ShapeSphere &in)", asMETHODPR(CShapeSphere, operator=, (const CShapeSphere&), CShapeSphere&), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectProperty("ShapeSphere", "float radius", asOFFSET(JPH::SphereShapeSettings, mRadius), asOFFSET(CShapeSphere, params), true); assert(r >= 0);

    r = engine->RegisterObjectType("PhysBody", sizeof(CPhysBody), asOBJ_VALUE | asGetTypeTraits<CPhysBody>()); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("PhysBody", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CPhysBody::CreateDefault), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("PhysBody", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CPhysBody::Destroy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectMethod("PhysBody", "PhysBody &opAssign(const PhysBody &in)", asMETHODPR(CPhysBody, operator=, (CPhysBody&&), CPhysBody&), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("PhysBody", "void InitFromShape(Scene::EntityId id, const ShapeSphere &in)", asFUNCTION(CPhysBody::InitFromShape<CShapeSphere>), asCALL_CDECL_OBJFIRST); assert(r >= 0);

    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}
