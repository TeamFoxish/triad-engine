#include "ResourceHandle.h"

#include "scripts/ScriptSystem.h"
#include "SharedStorage.h"
#include "resource/ResourceSystem.h"
#include "logs/Logs.h"

#include <new>

void CResourceHandle::ApplyOverrides(const YAML::Node& _tag)
{
	if (!_tag.IsScalar()) {
		return;
	}
	if (_tag.Scalar().empty()) {
		tag = Strid();
	}
	tag = ToStrid(_tag.Scalar());
}

void CResourceHandleDynamic::ApplyOverrides(const YAML::Node& _tag)
{
	if (!_tag.IsScalar() || _tag.Scalar().empty()) {
		return;
	}
	tagStr = _tag.Scalar();
}

static std::string gTempResTag;
void* CResourceHandleDynamic::Load() const
{
	if (!objType || tagStr.empty()) {
		return nullptr;
	}
	if (objType->GetSubTypeCount() == 0) {
		LOG_ERROR("SoftRef<T> should have at least one subtype T to work properly");
		return nullptr;
	}
	ResPath subpath;
	if (Triad::Resource::IsTagDynamic(tagStr)) {
		subpath = Triad::Resource::GetTagDynamicSubpath(ToStrid(tagStr));
	} else if (Triad::Resource::IsTag(tagStr)) {
		LOG_WARN("resource tag `{}` wasn't dynamic. check if it starts with `{}`", tagStr, Triad::Resource::TAG_DYNAMIC_PREFIX);
		subpath = Triad::Resource::GetTagSubpath(ToStrid(tagStr));
	} else {
		LOG_ERROR("failed to load dynamic resource `{}`. incorrect tag value. check if it starts with `{}`", tagStr, Triad::Resource::TAG_DYNAMIC_PREFIX);
		return nullptr;
	}
	ResTag tag = Triad::Resource::MakeTag(subpath);
	std::string _tagStr(tag.string());
	gResourceSys->LoadResource(tag);
	asITypeInfo* subtype = objType->GetSubType();
	asIScriptFunction* factory = static_cast<asIScriptFunction*>(subtype->GetUserData());
	const char* funcName = factory->GetName();
	assert(factory); // factory function ptr should be set to slot 0 for all resource handles to allow for native dynamic construction
	//ScriptObject handle(subtype, {{gScriptSys->GetStringType()->GetTypeId(), &_tagStr}}, factory);
	//ScriptObject handle(subtype, {}, factory);
	gTempResTag = _tagStr; // null pointer exception when calling native constructor, so apply shitcode here
	ScriptObject handle(subtype);
	return handle.GetRaw();
}

void CRenderable::ApplyOverrides(const YAML::Node& overrides)
{
	const YAML::Node& meshH = overrides["mesh"];
	if (meshH) {
		Set_mesh(CMeshHandle(ToStrid(meshH.Scalar())));
	}
	const YAML::Node& materialH = overrides["material"];
	if (materialH) {
		Set_material(CMaterialHandle(ToStrid(materialH.Scalar())));
	}
}

// TODO: replace constructors with common mixin
static void CResourceHandleDefaultConstructor(CResourceHandle* self)
{
	// temp shitcode
	if (gTempResTag.empty()) {
		new(self) CResourceHandle();
	} else {
		new(self) CResourceHandle(ToStrid(gTempResTag));
		gTempResTag.clear();
	}
}

static void CResourceHandleTagConstructor(const std::string& tag, CResourceHandle* self)
{
	new(self) CResourceHandle(ToStrid(tag));
}

static void CResourceHandleDestructor(CResourceHandle* self)
{
	static_cast<CResourceHandle*>(self)->~CResourceHandle();
}

static void CDynamicHandleTemplateConstructor(asITypeInfo* objType, CResourceHandleDynamic* self)
{
	new(self) CResourceHandleDynamic(objType);
}

static void CDynamicHandleTagConstructor(asITypeInfo* objType, const std::string& tag, CResourceHandleDynamic* self)
{
	new(self) CResourceHandleDynamic(objType, tag);
}

static void CDynamicHandleDestructor(CResourceHandleDynamic* self)
{
	static_cast<CResourceHandleDynamic*>(self)->~CResourceHandleDynamic();
}

static void CMaterialHandleDefaultConstructor(CMaterialHandle* self)
{
	// temp shitcode
	if (gTempResTag.empty()) {
		new(self) CMaterialHandle();
	} else {
		new(self) CMaterialHandle(ToStrid(gTempResTag));
		gTempResTag.clear();
	}
}

static void CMaterialHandleCopyConstructor(const CMaterialHandle& other, CMaterialHandle* self)
{
	// TODO: call destructor?
	new(self) CMaterialHandle(other);
}

static void CMaterialHandleTagConstructor(const std::string& tag, CMaterialHandle* self)
{
	new(self) CMaterialHandle(ToStrid(tag));
}

static void CMaterialDestructor(CMaterialHandle* self)
{
	static_cast<CMaterialHandle*>(self)->~CMaterialHandle();
}

static void CMeshHandleDefaultConstructor(CMeshHandle* self)
{
	// temp shitcode
	if (gTempResTag.empty()) {
		new(self) CMeshHandle();
	} else {
		new(self) CMeshHandle(ToStrid(gTempResTag));
		gTempResTag.clear();
	}
}

static void CMeshHandleCopyConstructor(const CMeshHandle& other, CMeshHandle* self)
{
	new(self) CMeshHandle(other);
}

static void CMeshHandleTagConstructor(const std::string& tag, CMeshHandle* self)
{
	new(self) CMeshHandle(ToStrid(tag));
}

static void CMeshDestructor(CMeshHandle* self)
{
	static_cast<CMeshHandle*>(self)->~CMeshHandle();
}

static void CRenderableDefaultConstructor(CRenderable* self)
{
	new(self) CRenderable();
}

static void CRenderableConstructor(asQWORD entityId, const CTransformHandle& transform, CRenderable* self)
{
	extern int32_t GetEntityInt32(asQWORD id);
	new(self) CRenderable(GetEntityInt32(entityId), transform);
}

static void CRenderableCopyConstructor(CRenderable& other, CRenderable* self)
{
	new(self) CRenderable(other);
}

static void CRenderableDestructor(CRenderable* self)
{
	static_cast<CRenderable*>(self)->~CRenderable();
}

void RegisterResourceHandles()
{
	auto engine = gScriptSys->GetRawEngine();
	int r;

	{
		const int typeId = r = 
			engine->RegisterObjectType("ResourceHandle", sizeof(CResourceHandle), asOBJ_VALUE | asGetTypeTraits<CResourceHandle>()); assert(r >= 0);
		CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
		const int funcId = r = engine->RegisterObjectBehaviour("ResourceHandle", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CResourceHandleDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
		//const int funcId = r = 
			engine->RegisterObjectBehaviour("ResourceHandle", asBEHAVE_CONSTRUCT, "void f(const string &in)", asFUNCTION(CResourceHandleTagConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("ResourceHandle", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CResourceHandleDestructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("ResourceHandle", "ResourceHandle &opAssign(const ResourceHandle &in)", asMETHODPR(CResourceHandle, operator=, (const CResourceHandle&), CResourceHandle&), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("ResourceHandle", "bool IsValid() const", asMETHOD(CResourceHandle, IsValid), asCALL_THISCALL); assert(r >= 0);
	
		asIScriptFunction* factoryPtr = engine->GetFunctionById(funcId);
		asITypeInfo* typeInfo = engine->GetTypeInfoById(typeId);
		typeInfo->SetUserData(factoryPtr);
	}

	// TODO: add ref on assign, sub ref on destruct
	r = engine->RegisterObjectType("SoftRef<class T>", sizeof(CResourceHandleDynamic), asOBJ_VALUE | asOBJ_TEMPLATE | asGetTypeTraits<CResourceHandleDynamic>()); assert(r >= 0);
	CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
	r = engine->RegisterObjectBehaviour("SoftRef<T>", asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTION(CDynamicHandleTemplateConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("SoftRef<T>", asBEHAVE_CONSTRUCT, "void f(int&in, const string &in)", asFUNCTION(CDynamicHandleTagConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("SoftRef<T>", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CDynamicHandleDestructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("SoftRef<T>", "SoftRef<T> &opAssign(const SoftRef<T> &in)", asMETHODPR(CResourceHandleDynamic, operator=, (const CResourceHandleDynamic&), CResourceHandleDynamic&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("SoftRef<T>", "T& Load() const", asMETHOD(CResourceHandleDynamic, Load), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("SoftRef<T>", "bool IsValid() const", asMETHOD(CResourceHandleDynamic, IsValid), asCALL_THISCALL); assert(r >= 0);

	r = engine->RegisterObjectType("Mesh", sizeof(CMeshHandle), asOBJ_VALUE | asGetTypeTraits<CMeshHandle>()); assert(r >= 0);
	CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
	r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CMeshHandleDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_CONSTRUCT, "void f(const Mesh &in)", asFUNCTION(CMeshHandleCopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_CONSTRUCT, "void f(const string &in)", asFUNCTION(CMeshHandleTagConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CMeshDestructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Mesh", "Mesh &opAssign(const Mesh &in)", asMETHODPR(CMeshHandle, operator=, (const CMeshHandle&), CMeshHandle&), asCALL_THISCALL); assert(r >= 0);

	r = engine->RegisterObjectType("Material", sizeof(CMaterialHandle), asOBJ_VALUE | asGetTypeTraits<CMaterialHandle>()); assert(r >= 0);
	CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
	r = engine->RegisterObjectBehaviour("Material", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CMaterialHandleDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Material", asBEHAVE_CONSTRUCT, "void f(const Material &in)", asFUNCTION(CMaterialHandleCopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Material", asBEHAVE_CONSTRUCT, "void f(const string &in)", asFUNCTION(CMaterialHandleTagConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Material", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CMaterialDestructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Material", "Material &opAssign(const Material &in)", asMETHODPR(CMaterialHandle, operator=, (const CMaterialHandle&), CMaterialHandle&), asCALL_THISCALL); assert(r >= 0);

	r = engine->RegisterObjectType("Renderable", sizeof(CRenderable), asOBJ_VALUE | asGetTypeTraits<CRenderable>() | asOBJ_APP_CLASS_MORE_CONSTRUCTORS); assert(r >= 0);
	CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
	r = engine->RegisterObjectBehaviour("Renderable", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CRenderableDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Renderable", asBEHAVE_CONSTRUCT, "void f(uint64, const Math::Transform@+)", asFUNCTION(CRenderableConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Renderable", asBEHAVE_CONSTRUCT, "void f(Renderable &in)", asFUNCTION(CRenderableCopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Renderable", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CRenderableDestructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "Renderable &opAssign(Renderable &in)", asMETHODPR(CRenderable, operator=, (CRenderable&), CRenderable&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "Material& get_material() property", asMETHOD(CRenderable, Get_material), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "void set_material(Material &in) property", asMETHOD(CRenderable, Set_material), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "Mesh& get_mesh() property", asMETHOD(CRenderable, Get_mesh), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "void set_mesh(Mesh &in) property", asMETHOD(CRenderable, Set_mesh), asCALL_THISCALL); assert(r >= 0);
}
