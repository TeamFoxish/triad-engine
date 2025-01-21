#include "LightsBindings.h"

#include "LightsStorage.h"
#include "shared/MathScriptBindings.h"
#include "shared/Shared.h"
#include "scripts/ScriptSystem.h"

#include <new>

template<typename ChildT, typename LightT>
class CLightSource {
public:
    using ParentT = CLightSource<ChildT, LightT>;
    using StorageT = LightsStorage::StorageImpl<LightT>;

    CLightSource() = default;

    CLightSource(TransformStorage::Handle transform)
    {
        typename StorageT::LightSource light;
        light.light = std::make_unique<LightT>();
        light.transform = transform;
        handle = StorageT::Instance().Add(std::move(light));
    }

    CLightSource(const ChildT& other)
    {
        *this = other;
    }

    ~CLightSource()
    {
        StorageT::Instance().Remove(handle);
    }

    ChildT& operator=(const ChildT& other) 
    {
        typename StorageT::LightSource& otherLightSrc = StorageT::Instance().GetLightSource(other.handle);
        typename StorageT::LightSource lightSrc;
        lightSrc.light = std::make_unique<LightT>();
        *lightSrc.light = *otherLightSrc.light;
        lightSrc.transform = otherLightSrc.transform;
        handle = StorageT::Instance().Add(std::move(lightSrc));
        return *static_cast<ChildT*>(this);
    }

    LightT& GetLight() const { return StorageT::Instance().Get(handle); }

    static Math::Vector3 GetColor(const ChildT* self) { return self->GetLight().GetColor().ToVector3(); }
    static void SetColor(ChildT* self, const Math::Vector3& color) { self->GetLight().SetColor(Math::Color(color)); }

    static void CreateDefault(ChildT* self) { new(self) ChildT(); }
    static void CreateFromTransform(ChildT* self, const CTransformHandle& transform) { new(self) ChildT(transform.GetHandle()); }
    static void CreateCopy(ChildT* self, const ChildT& other) { new(self) ChildT(other); }
    static void Destroy(ChildT* self) { self->~ChildT(); }

protected:
    StorageT::Handle handle;
};

class CDirectionalLight : public CNativeObject, public CLightSource<CDirectionalLight, DirectionalLight> {
public:
    CDirectionalLight() = default;
    CDirectionalLight(TransformStorage::Handle transform) 
        : ParentT(transform)
    {
    }
    CDirectionalLight(const CDirectionalLight& other)
        : ParentT(other)
    {
    }

    CDirectionalLight& operator=(const CDirectionalLight& other)
    {
        ParentT::operator=(other);
        return *this;
    }

    void ApplyOverrides(const YAML::Node& overrides) override;

    YAML::Node Serialize() const override;
};

static float ParseFloat(const YAML::Node& node) 
{
    if (!node.IsScalar() || node.Scalar().empty()) {
        return 0.0f;
    }
    return node.as<float>();
}

void CDirectionalLight::ApplyOverrides(const YAML::Node& overrides)
{
    if (const YAML::Node& col = overrides["color"]) {
        Math::Vector3 colVal = GetColor(this);
        colVal.x = col["b"] ? ParseFloat(col["b"]): colVal.x;
        colVal.y = col["g"] ? ParseFloat(col["g"]): colVal.y;
        colVal.z = col["r"] ? ParseFloat(col["r"]): colVal.z;
        SetColor(this, colVal);
    }
}

YAML::Node CDirectionalLight::Serialize() const
{
    YAML::Node res;
    YAML::Node& col = res["color"] = YAML::Node();
    const Math::Vector3 colVal = GetColor(this);
    col["r"] = colVal.z;
    col["g"] = colVal.y;
    col["b"] = colVal.x;
    return res;
}

class CPointLight : public CNativeObject, public CLightSource<CPointLight, PointLight> {
public:
    CPointLight() = default;
    CPointLight(TransformStorage::Handle transform)
        : ParentT(transform)
    {
    }
    CPointLight(const CPointLight& other) 
        : ParentT(other)
    {
    }

    CPointLight& operator=(const CPointLight& other) 
    {
        ParentT::operator=(other);
        return *this;
    }

    float GetIntensity() const { return GetLight().GetIntensity(); }
    void SetIntensity(float intensity) { GetLight().SetIntensity(intensity); }

    float GetRadius() const { return GetLight().GetRadius(); }
    void SetRadius(float radius) { return GetLight().SetRadius(radius); }

    void ApplyOverrides(const YAML::Node& overrides) override;

    YAML::Node Serialize() const override;
};

void CPointLight::ApplyOverrides(const YAML::Node& overrides)
{
    PointLight& light = GetLight();
    if (const YAML::Node& col = overrides["color"]) {
        Math::Vector3 colVal = GetColor(this);
        colVal.x = col["b"] ? ParseFloat(col["b"]) : colVal.x;
        colVal.y = col["g"] ? ParseFloat(col["g"]) : colVal.y;
        colVal.z = col["r"] ? ParseFloat(col["r"]) : colVal.z;
        light.SetColor(Math::Color(colVal));
    }
    if (const YAML::Node& intense = overrides["intensity"]) {
        light.SetIntensity(ParseFloat(intense));
    }
    if (const YAML::Node& radius = overrides["radius"]) {
        light.SetRadius(ParseFloat(radius));
    }
}

YAML::Node CPointLight::Serialize() const
{
    YAML::Node res;
    const PointLight& light = GetLight();
    YAML::Node& col = res["color"] = YAML::Node();
    const Math::Color colVal = light.GetColor();
    col["r"] = colVal.z;
    col["g"] = colVal.y;
    col["b"] = colVal.x;
    res["intensity"] = light.GetIntensity();
    res["radius"] = light.GetRadius();
    return res;
}

void RegisterLightsBindings()
{
    auto engine = gScriptSys->GetRawEngine();
    int r;

    r = engine->RegisterObjectType("DirectionalLight", sizeof(CDirectionalLight), asOBJ_VALUE | asGetTypeTraits<CDirectionalLight>() | asOBJ_APP_CLASS_MORE_CONSTRUCTORS); assert(r >= 0);
    CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
    r = engine->RegisterObjectBehaviour("DirectionalLight", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CDirectionalLight::CreateDefault), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("DirectionalLight", asBEHAVE_CONSTRUCT, "void f(const Math::Transform@+)", asFUNCTION(CDirectionalLight::CreateFromTransform), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("DirectionalLight", asBEHAVE_CONSTRUCT, "void f(const DirectionalLight &in)", asFUNCTION(CDirectionalLight::CreateCopy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("DirectionalLight", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CDirectionalLight::Destroy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectMethod("DirectionalLight", "DirectionalLight &opAssign(const DirectionalLight &in)", asMETHODPR(CDirectionalLight, operator=, (const CDirectionalLight&), CDirectionalLight&), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("DirectionalLight", "Math::Vector3 GetColor() const", asFUNCTION(CDirectionalLight::GetColor), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectMethod("DirectionalLight", "void SetColor(const Math::Vector3 &in color)", asFUNCTION(CDirectionalLight::SetColor), asCALL_CDECL_OBJFIRST); assert(r >= 0);

    r = engine->RegisterObjectType("PointLight", sizeof(CPointLight), asOBJ_VALUE | asGetTypeTraits<CPointLight>() | asOBJ_APP_CLASS_MORE_CONSTRUCTORS); assert(r >= 0);
    CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
    r = engine->RegisterObjectBehaviour("PointLight", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CPointLight::CreateDefault), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("PointLight", asBEHAVE_CONSTRUCT, "void f(const Math::Transform@+)", asFUNCTION(CPointLight::CreateFromTransform), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("PointLight", asBEHAVE_CONSTRUCT, "void f(const PointLight &in)", asFUNCTION(CPointLight::CreateCopy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("PointLight", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CPointLight::Destroy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectMethod("PointLight", "PointLight &opAssign(const PointLight &in)", asMETHODPR(CPointLight, operator=, (const CPointLight&), CPointLight&), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("PointLight", "Math::Vector3 GetColor() const", asFUNCTION(CPointLight::GetColor), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectMethod("PointLight", "void SetColor(const Math::Vector3 &in color)", asFUNCTION(CPointLight::SetColor), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectMethod("PointLight", "float GetIntensity() const", asMETHOD(CPointLight, GetIntensity), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("PointLight", "void SetIntensity(float intensity)", asMETHOD(CPointLight, SetIntensity), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("PointLight", "float GetRadius() const", asMETHOD(CPointLight, GetRadius), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("PointLight", "void SetRadius(float radius)", asMETHOD(CPointLight, SetRadius), asCALL_THISCALL); assert(r >= 0);
}
