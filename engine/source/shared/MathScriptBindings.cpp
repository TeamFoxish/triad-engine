#include "MathScriptBindings.h"

#include "scripts/ScriptSystem.h"
#include "SharedStorage.h"

#include <new>

#pragma region REF
void CRef::AddRef()
{
	++refCount;
}

void CRef::Release()
{
	if (--refCount == 0) {
		delete this;
	}
}
#pragma endregion

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
}
#pragma endregion

#pragma region TRANSFORM
CTransformHandle* STransformHandleFactory()
{
	return new CTransformHandle();
}

CTransformHandle::CTransformHandle()
	: handle(SharedStorage::Instance().transforms.Add())
{
}

CTransformHandle::~CTransformHandle()
{
	SharedStorage::Instance().transforms.Remove(handle);
}

const Math::Transform& CTransformHandle::GetTransform() const
{
	return SharedStorage::Instance().transforms.AccessRead(handle);
}

Math::Transform& CTransformHandle::GetTransform()
{
	return SharedStorage::Instance().transforms.AccessWrite(handle);
}

void RegisterTransform() 
{
	auto engine = gScriptSys->GetRawEngine();
	int r = engine->RegisterObjectType("Transform", 0, asOBJ_REF); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_FACTORY, "Transform@ f()", asFUNCTION(STransformHandleFactory), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_ADDREF, "void f()", asMETHOD(CTransformHandle, AddRef), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Transform", asBEHAVE_RELEASE, "void f()", asMETHOD(CTransformHandle, Release), asCALL_THISCALL); assert(r >= 0);

	r = engine->RegisterObjectMethod("Transform", "Vector3 GetPosition()", asMETHOD(CTransformHandle, GetPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetPosition(const Vector3 &in)", asMETHOD(CTransformHandle, SetPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "Vector3 GetScale()", asMETHOD(CTransformHandle, GetScale), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetScale(const Vector3 &in)", asMETHOD(CTransformHandle, SetScale), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "Vector3 GetLocalPosition()", asMETHOD(CTransformHandle, GetLocalPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetLocalPosition(const Vector3 &in)", asMETHOD(CTransformHandle, SetLocalPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "Vector3 GetLocalScale()", asMETHOD(CTransformHandle, GetLocalScale), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Transform", "void SetLocalScale(const Vector3 &in)", asMETHOD(CTransformHandle, SetLocalScale), asCALL_THISCALL); assert(r >= 0);
	// TODO: add quaternions bindings
}
#pragma endregion

bool MathScriptBindingsInit()
{
	RegisterVector3();
	RegisterTransform();

	return true;
}

void MathScriptBindingsTerm()
{
}
