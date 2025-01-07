#include "MathScriptBindings.h"

#include "scripts/ScriptSystem.h"
#include "SharedStorage.h"

#include <new>

#pragma region VECTOR3
static void Vector3DefaultConstructor(Math::Vector3* self)
{
	new(self) Math::Vector3();
}

static void Vector3CopyConstructor(const Math::Vector3& other, Math::Vector3* self)
{
	new(self) Math::Vector3(other);
}

static void Vector3InitConstructor(float x, float y, float z, Math::Vector3* self)
{
	new(self) Math::Vector3(x, y, z);
}

static float Vector3Dot(const Math::Vector3& a, const Math::Vector3& b)
{
	return a.Dot(b);
}

static Math::Vector3 Vector3Cross(const Math::Vector3& a, const Math::Vector3& b)
{
	return a.Cross(b);
}

static Math::Vector3 Vector3Clamp(const Math::Vector3& v, const Math::Vector3& min, const Math::Vector3& max)
{
	Math::Vector3 res;
	v.Clamp(min, max, res);
	return res;
}

static Math::Vector3 Vector3NormalizedCopy(Math::Vector3* self)
{
	Math::Vector3 res = *self;
	res.Normalize();
	return res;
}

static Math::Vector3 Vector3TransformCopy(const Math::Vector3* self, const Math::Quaternion& quat)
{
	return Math::Vector3::Transform(*self, quat);
}

static void Vector3Transform(Math::Vector3* self, const Math::Quaternion& quat)
{
	const Math::Vector3 temp = *self;
	Math::Vector3::Transform(temp, quat, *self);
}

static void RegisterVector3()
{
	using namespace DirectX;
	using namespace Math;
	auto engine = gScriptSys->GetRawEngine();
	int r;

	// Register the type
	r = engine->RegisterObjectType("Vector3", sizeof(Vector3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);

	// Register the object properties
	r = engine->RegisterObjectProperty("Vector3", "float x", asOFFSET(Vector3, x)); assert(r >= 0);
	r = engine->RegisterObjectProperty("Vector3", "float y", asOFFSET(Vector3, y)); assert(r >= 0);
	r = engine->RegisterObjectProperty("Vector3", "float z", asOFFSET(Vector3, z)); assert(r >= 0);

	// Register the constructors
	r = engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector3DefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in)", asFUNCTION(Vector3CopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float, float y = 0, float z = 0)", asFUNCTION(Vector3InitConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);

	// Register the operator overloads
	r = engine->RegisterObjectMethod("Vector3", "Vector3 &opAddAssign(const Vector3 &in)", asMETHODPR(Vector3, operator+=, (const Vector3&), Vector3&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 &opSubAssign(const Vector3 &in)", asMETHODPR(Vector3, operator-=, (const Vector3&), Vector3&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 &opMulAssign(float)", asMETHODPR(Vector3, operator*=, (float), Vector3&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 &opDivAssign(float)", asMETHODPR(Vector3, operator/=, (float), Vector3&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 opNeg()", asMETHODPR(Vector3, operator-, () const noexcept, Vector3), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "bool opEquals(const Vector3 &in) const", asMETHODPR(Vector3, operator==, (const Vector3&) const noexcept, bool), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 opAdd(const Vector3 &in) const", asFUNCTIONPR(operator+, (const Vector3&, const Vector3&) noexcept, Vector3), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 opSub(const Vector3 &in) const", asFUNCTIONPR(operator-, (const Vector3&, const Vector3&), Vector3), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 opMul(float) const", asFUNCTIONPR(operator*, (const Vector3&, float), Vector3), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 opMul_r(float) const", asFUNCTIONPR(operator*, (float, const Vector3&), Vector3), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 opDiv(float) const", asFUNCTIONPR(operator/, (const Vector3&, float), Vector3), asCALL_CDECL_OBJFIRST); assert(r >= 0);

	// Register the object methods
	r = engine->RegisterObjectMethod("Vector3", "float Length() const", asMETHOD(Vector3, Length), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "float LengthSq() const", asMETHOD(Vector3, LengthSquared), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "void Clamp(const Vector3 &in min, const Vector3 &in max)", asMETHODPR(Vector3, Clamp, (const Vector3&, const Vector3&), void), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 Cross(const Vector3 &in other)", asMETHODPR(Vector3, Cross, (const Vector3&) const noexcept, Vector3), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "float Dot(const Vector3 &in other)", asMETHOD(Vector3, Dot), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "void Normalize()", asMETHOD(Vector3, Normalize), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 NormalizedCopy() const", asFUNCTION(Vector3NormalizedCopy), asCALL_CDECL_OBJLAST); assert(r >= 0);

	// Register global functions
	r = engine->RegisterGlobalFunction("float Vector3Distance(const Vector3 &in a, const Vector3 &in b)", asFUNCTION(Vector3::Distance), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Vector3DistanceSq(const Vector3 &in a, const Vector3 &in b)", asFUNCTION(Vector3::DistanceSquared), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Vector3 Vector3Reflect(const Vector3 &in vec, const Vector3 &in normal)", asFUNCTIONPR(Vector3::Reflect, (const Vector3&, const Vector3&), Vector3), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Vector3 Lerp(const Vector3 &in a, const Vector3 &in b, float t)", asFUNCTIONPR(Vector3::Lerp, (const Vector3&, const Vector3&, float), Vector3), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Vector3 SmoothStep(const Vector3 &in a, const Vector3 &in b, float t)", asFUNCTIONPR(Vector3::SmoothStep, (const Vector3&, const Vector3&, float), Vector3), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Vector3 Max(const Vector3 &in a, const Vector3 &in b)", asFUNCTIONPR(Vector3::Max, (const Vector3&, const Vector3&), Vector3), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Vector3 Min(const Vector3 &in a, const Vector3 &in b)", asFUNCTIONPR(Vector3::Min, (const Vector3&, const Vector3&), Vector3), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Dot(const Vector3 &in a, const Vector3 &in b)", asFUNCTION(Vector3Dot), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Vector3 Cross(const Vector3 &in a, const Vector3 &in b)", asFUNCTION(Vector3Cross), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Vector3 Clamp(const Vector3 &in v, const Vector3 &in min, const Vector3 &in max)", asFUNCTION(Vector3Clamp), asCALL_CDECL); assert(r >= 0);

	// Register global properties
	r = engine->RegisterGlobalProperty("const Vector3 Vector3Forward", const_cast<Vector3*>(&Vector3::Forward));
	r = engine->RegisterGlobalProperty("const Vector3 Vector3Right", const_cast<Vector3*>(&Vector3::Right));
	r = engine->RegisterGlobalProperty("const Vector3 Vector3Up", const_cast<Vector3*>(&Vector3::Up));
	r = engine->RegisterGlobalProperty("const Vector3 Vector3Zero", const_cast<Vector3*>(&Vector3::Zero));
}
#pragma endregion

#pragma region QUATERNION
static void QuaternionDefaultConstructor(Math::Quaternion* self)
{
	new(self) Math::Quaternion();
}

static void QuaternionCopyConstructor(const Math::Quaternion& other, Math::Quaternion* self)
{
	new(self) Math::Quaternion(other);
}

static void QuaternionInitConstructor(float x, float y, float z, float w, Math::Quaternion* self)
{
	new(self) Math::Quaternion(x, y, z, w);
}

static void RegisterQaternion()
{
	using namespace DirectX;
	using namespace Math;
	auto engine = gScriptSys->GetRawEngine();
	int r;

	// Register the type
	r = engine->RegisterObjectType("Quaternion", sizeof(Quaternion), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);

	// Register the object properties
	r = engine->RegisterObjectProperty("Quaternion", "float x", asOFFSET(Quaternion, x)); assert(r >= 0);
	r = engine->RegisterObjectProperty("Quaternion", "float y", asOFFSET(Quaternion, y)); assert(r >= 0);
	r = engine->RegisterObjectProperty("Quaternion", "float z", asOFFSET(Quaternion, z)); assert(r >= 0);
	r = engine->RegisterObjectProperty("Quaternion", "float w", asOFFSET(Quaternion, w)); assert(r >= 0);

	// Register the constructors
	r = engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(QuaternionDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(const Quaternion &in)", asFUNCTION(QuaternionCopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z, float w)", asFUNCTION(QuaternionInitConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);

	// axis directions are fucked up :)
	r = engine->RegisterGlobalFunction("Quaternion QuaternionFromYawPitchRoll(float yaw, float pitch, float roll)", asFUNCTIONPR(Quaternion::CreateFromYawPitchRoll, (float, float, float), Quaternion), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Quaternion QuaternionFromAxisAngle(const Vector3 &in axis, float angle)", asFUNCTION(Quaternion::CreateFromAxisAngle), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Quaternion QuaternionFromToRotation(const Vector3 &in a, const Vector3 &in b)", asFUNCTIONPR(Quaternion::FromToRotation, (const Vector3&, const Vector3&), Quaternion), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Quaternion QuaternionFromLookRotation(const Vector3 &in forward, const Vector3 &in up)", asFUNCTIONPR(Quaternion::LookRotation, (const Vector3&, const Vector3&), Quaternion), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Quaternion QuaternionConcatenate(const Quaternion &in a, const Quaternion &in b)", asFUNCTIONPR(Quaternion::Concatenate, (const Quaternion&, const Quaternion&), Quaternion), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Quaternion Lerp(const Quaternion &in a, const Quaternion &in b, float t)", asFUNCTIONPR(Quaternion::Lerp, (const Quaternion&, const Quaternion&, float), Quaternion), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("Quaternion Slerp(const Quaternion &in a, const Quaternion &in b, float t)", asFUNCTIONPR(Quaternion::Slerp, (const Quaternion&, const Quaternion&, float), Quaternion), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("float QuaternionAngle(const Quaternion &in a, const Quaternion &in b)", asFUNCTION(Quaternion::Angle), asCALL_CDECL); assert(r >= 0);

	r = engine->RegisterGlobalProperty("const Quaternion QuaternionIdentity", const_cast<Quaternion*>(&Quaternion::Identity));

	// Register the operator overloads
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion &opAddAssign(const Quaternion &in)", asMETHODPR(Quaternion, operator+=, (const Quaternion&), Quaternion&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion &opSubAssign(const Quaternion &in)", asMETHODPR(Quaternion, operator-=, (const Quaternion&), Quaternion&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion &opMulAssign(const Quaternion &in)", asMETHODPR(Quaternion, operator*=, (const Quaternion&), Quaternion&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion &opMulAssign(float)", asMETHODPR(Quaternion, operator*=, (float), Quaternion&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion &opDivAssign(const Quaternion &in)", asMETHODPR(Quaternion, operator/=, (const Quaternion&), Quaternion&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion opNeg()", asMETHODPR(Quaternion, operator-, () const noexcept, Quaternion), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "bool opEquals(const Quaternion &in) const", asMETHODPR(Quaternion, operator==, (const Quaternion&) const noexcept, bool), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion opAdd(const Quaternion &in) const", asFUNCTIONPR(operator+, (const Quaternion&, const Quaternion&) noexcept, Quaternion), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion opSub(const Quaternion &in) const", asFUNCTIONPR(operator-, (const Quaternion&, const Quaternion&), Quaternion), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion opMul(const Quaternion &in) const", asFUNCTIONPR(operator*, (const Quaternion&, const Quaternion&), Quaternion), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion opMul(float) const", asFUNCTIONPR(operator*, (const Quaternion&, float), Quaternion), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion opMul_r(float) const", asFUNCTIONPR(operator*, (float, const Quaternion&), Quaternion), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Quaternion opDiv(const Quaternion &in) const", asFUNCTIONPR(operator/, (const Quaternion&, const Quaternion&), Quaternion), asCALL_CDECL_OBJFIRST); assert(r >= 0);

	// Register the object methods
	r = engine->RegisterObjectMethod("Quaternion", "float Length() const", asMETHOD(Quaternion, Length), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "float LengthSq() const", asMETHOD(Quaternion, LengthSquared), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "void Inverse(Quaternion &out) const", asMETHOD(Quaternion, Inverse), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "Vector3 ToEuler() const", asMETHOD(Quaternion, ToEuler), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "void RotateTowards(const Quaternion &in, float maxAngle)", asMETHODPR(Quaternion, RotateTowards, (const Quaternion&, float), void), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Quaternion", "void Normalize()", asMETHODPR(Quaternion, Normalize, (), void), asCALL_THISCALL); assert(r >= 0);

	// Register Vector3 Transform
	r = engine->RegisterGlobalFunction("Vector3 Vector3Transform(const Vector3 &in v, const Quaternion &in quat)", asFUNCTIONPR(Vector3::Transform, (const Vector3&, const Quaternion&), Vector3), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "Vector3 Transform(const Quaternion &in quat) const", asFUNCTION(Vector3TransformCopy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector3", "void Transform(const Quaternion &in quat)", asFUNCTION(Vector3Transform), asCALL_CDECL_OBJFIRST); assert(r >= 0);
}
#pragma endregion

#pragma region TRANSFORM
CTransformHandle* STransformHandleFactory(const CTransformHandle* parent = nullptr)
{
	return new CTransformHandle(parent);
}

CTransformHandle::CTransformHandle(const CTransformHandle* parent)
{
	if (parent) {
		handle = SharedStorage::Instance().transforms.Add(parent->handle);
		return;
	}
	handle = SharedStorage::Instance().transforms.Add();
}

CTransformHandle::~CTransformHandle()
{
	SharedStorage::Instance().transforms.Remove(handle);
}

CTransformHandle* STransformHandleAssign(const CTransformHandle* other, CTransformHandle* self)
{
	self->GetTransform() = other->GetTransform();
	return self;
}

const Math::Transform& CTransformHandle::GetTransform() const
{
	return SharedStorage::Instance().transforms.AccessRead(handle);
}

Math::Transform& CTransformHandle::GetTransform()
{
	return SharedStorage::Instance().transforms.AccessWrite(handle);
}


// Here place for to string callbacks for debugger
#ifdef EDITOR
std::string TransformToString(void *obj, int expandMembersLevel, CDebugger* dbg) {
	CTransformHandle* transformHandle = static_cast<CTransformHandle*>(obj);
	DirectX::SimpleMath::Vector3 position = transformHandle->GetPosition();
	DirectX::SimpleMath::Vector3 rotation = transformHandle->GetRotation().ToEuler();
	DirectX::SimpleMath::Vector3 scale = transformHandle->GetScale();
	return std::format("Position: {}, {}, {} Rotation: {}, {}, {}, Scale: {}, {}, {}", 
						position.x, position.y, position.z,
						rotation.x, rotation.y, rotation.z,
						scale.x, scale.y, scale.z);
}
#endif // EDITOR

void CTransformHandle::ApplyOverrides(const YAML::Node& overrides)
{
	Math::Vector3 pos = GetLocalPosition();
	Math::Vector3 rot = GetLocalRotation().ToEuler();
	Math::Vector3 scale = GetLocalScale();
	if (const YAML::Node& posVal = overrides["position"]) {
		pos.x = posVal["x"] ? posVal["x"].as<float>() : pos.x;
		pos.y = posVal["y"] ? posVal["y"].as<float>() : pos.y;
		pos.z = posVal["z"] ? posVal["z"].as<float>() : pos.z;
		SetLocalPosition(pos);
	}
	if (const YAML::Node& rotVal = overrides["rotation"]) {
		rot = Math::RadToDeg(rot);
		rot.x = rotVal["x"] ? rotVal["x"].as<float>() : rot.x;
		rot.y = rotVal["y"] ? rotVal["y"].as<float>() : rot.y;
		rot.z = rotVal["z"] ? rotVal["z"].as<float>() : rot.z;
		rot = Math::DegToRad(rot);
		SetLocalRotation(Math::Quaternion::CreateFromYawPitchRoll(rot.x, rot.y, rot.z));
	}
	if (const YAML::Node& scaleVal = overrides["scale"]) {
		scale.x = scaleVal["x"] ? scaleVal["x"].as<float>() : scale.x;
		scale.y = scaleVal["y"] ? scaleVal["y"].as<float>() : scale.y;
		scale.z = scaleVal["z"] ? scaleVal["z"].as<float>() : scale.z;
		SetLocalScale(scale);
	}
}

static void RegisterTransform() 
{
	auto engine = gScriptSys->GetRawEngine();
	int r = engine->RegisterObjectType("Transform", 0, asOBJ_REF); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_FACTORY, "Transform@ f(const Transform@+ parent = null)", asFUNCTION(STransformHandleFactory), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_ADDREF, "void f()", asMETHOD(CTransformHandle, AddRef), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_RELEASE, "void f()", asMETHOD(CTransformHandle, Release), asCALL_THISCALL); assert(r >= 0);

	r = engine->RegisterObjectMethod("Transform", "Transform@+ opAssign(const Transform@+)", asFUNCTION(STransformHandleAssign), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "Vector3 GetPosition() const", asMETHOD(CTransformHandle, GetPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetPosition(const Vector3 &in)", asMETHOD(CTransformHandle, SetPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "Quaternion GetRotation() const", asMETHOD(CTransformHandle, GetRotation), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetRotation(const Quaternion &in)", asMETHOD(CTransformHandle, SetRotation), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "Vector3 GetScale() const", asMETHOD(CTransformHandle, GetScale), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetScale(const Vector3 &in)", asMETHOD(CTransformHandle, SetScale), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "Vector3 GetLocalPosition() const", asMETHOD(CTransformHandle, GetLocalPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetLocalPosition(const Vector3 &in)", asMETHOD(CTransformHandle, SetLocalPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "Quaternion GetLocalRotation() const", asMETHOD(CTransformHandle, GetLocalRotation), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetLocalRotation(const Quaternion &in)", asMETHOD(CTransformHandle, SetLocalRotation), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "Vector3 GetLocalScale() const", asMETHOD(CTransformHandle, GetLocalScale), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetLocalScale(const Vector3 &in)", asMETHOD(CTransformHandle, SetLocalScale), asCALL_THISCALL); assert(r >= 0);

// Here place for to string callbacks bindings
#ifdef EDITOR
	CDebugger* debugger = gScriptSys->GetDebugger();

	asITypeInfo* transformType = engine->GetTypeInfoByDecl("Transform");

	debugger->RegisterToStringCallback(transformType, TransformToString);
#endif // EDITOR
}
#pragma endregion

#pragma region HELPERS
static void RegisterHelpers() 
{
	using namespace DirectX;
	using namespace Math;
	auto engine = gScriptSys->GetRawEngine();
	int r;

	// constants
	r = engine->RegisterGlobalProperty("const float Pi", const_cast<float*>(&Pi)); assert(r >= 0);

	// helpers
	r = engine->RegisterGlobalFunction("float DegToRad(float deg)", asFUNCTIONPR(DegToRad, (float), float), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("float RadToDeg(float rad)", asFUNCTIONPR(RadToDeg, (float), float), asCALL_CDECL); assert(r >= 0);

	r = engine->RegisterGlobalFunction("uint8 Max(uint8, uint8)", asFUNCTION(Max<asBYTE>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint16 Max(uint16, uint16)", asFUNCTION(Max<asWORD>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint Max(uint, uint)", asFUNCTION(Max<asUINT>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint64 Max(uint64, uint64)", asFUNCTION(Max<asQWORD>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int8 Max(int8, int8)", asFUNCTION(Max<asINT8>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int16 Max(int16, int16)", asFUNCTION(Max<asINT16>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int Max(int, int)", asFUNCTION(Max<asINT32>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 Max(int64, int64)", asFUNCTION(Max<asINT64>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Max(float, float)", asFUNCTION(Max<float>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double Max(double, double)", asFUNCTION(Max<double>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint8 Min(uint8, uint8)", asFUNCTION(Max<asBYTE>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint16 Min(uint16, uint16)", asFUNCTION(Max<asWORD>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint Min(uint, uint)", asFUNCTION(Max<asUINT>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint64 Min(uint64, uint64)", asFUNCTION(Max<asQWORD>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int8 Min(int8, int8)", asFUNCTION(Max<asINT8>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int16 Min(int16, int16)", asFUNCTION(Max<asINT16>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int Min(int, int)", asFUNCTION(Max<asINT32>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 Min(int64, int64)", asFUNCTION(Max<asINT64>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Min(float, float)", asFUNCTION(Max<float>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double Min(double, double)", asFUNCTION(Max<double>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint8 Clamp(uint8, uint8, uint8)", asFUNCTION(Clamp<asBYTE>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint16 Clamp(uint16, uint16, uint16)", asFUNCTION(Clamp<asWORD>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint Clamp(uint, uint, uint)", asFUNCTION(Clamp<asUINT>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint64 Clamp(uint64, uint64, uint64)", asFUNCTION(Clamp<asQWORD>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int8 Clamp(int8, int8, int8)", asFUNCTION(Clamp<asINT8>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int16 Clamp(int16, int16, int16)", asFUNCTION(Clamp<asINT16>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int Clamp(int, int, int)", asFUNCTION(Clamp<asINT32>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 Clamp(int64, int64, int64)", asFUNCTION(Clamp<asINT64>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Clamp(float, float, float)", asFUNCTION(Clamp<float>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double Clamp(double, double, double)", asFUNCTION(Clamp<double>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int8 Abs(int8)", asFUNCTION(Abs<asINT8>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int16 Abs(int16)", asFUNCTION(Abs<asINT16>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int Abs(int)", asFUNCTION(Abs<asINT32>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 Abs(int64)", asFUNCTION(Abs<asINT64>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Abs(float)", asFUNCTION(Abs<float>), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double Abs(double)", asFUNCTION(Abs<double>), asCALL_CDECL); assert(r >= 0);
}
#pragma endregion

bool MathScriptBindingsInit()
{
	auto engine = gScriptSys->GetRawEngine();
	int r;
	r = engine->SetDefaultNamespace("Math"); assert(r >= 0);

	RegisterVector3();
	RegisterQaternion();
	RegisterTransform();
	RegisterHelpers();

	r = engine->SetDefaultNamespace(""); assert(r >= 0);

	return true;
}

void MathScriptBindingsTerm()
{
}
