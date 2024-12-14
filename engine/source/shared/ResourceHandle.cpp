#include "ResourceHandle.h"

#include "scripts/ScriptSystem.h"
#include "SharedStorage.h"

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
static void CMaterialHandleDefaultConstructor(CMaterialHandle* self)
{
	new(self) CMaterialHandle();
}

static void CMaterialHandleCopyConstructor(const CMaterialHandle& other, CMaterialHandle* self)
{
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
	new(self) CMeshHandle();
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

static void CRenderableConstructor(uint32_t entityId, const CTransformHandle& transform, CRenderable* self)
{
	new(self) CRenderable(entityId, transform);
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
	r = engine->RegisterObjectType("Mesh", sizeof(CMeshHandle), asOBJ_VALUE | asGetTypeTraits<CMeshHandle>()); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CMeshHandleDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_CONSTRUCT, "void f(const Mesh &in)", asFUNCTION(CMeshHandleCopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_CONSTRUCT, "void f(const string &in)", asFUNCTION(CMeshHandleTagConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CMeshDestructor), asCALL_CDECL_OBJLAST); assert(r >= 0);

	r = engine->RegisterObjectType("Material", sizeof(CMaterialHandle), asOBJ_VALUE | asGetTypeTraits<CMaterialHandle>()); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Material", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CMaterialHandleDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Material", asBEHAVE_CONSTRUCT, "void f(const Material &in)", asFUNCTION(CMaterialHandleCopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Material", asBEHAVE_CONSTRUCT, "void f(const string &in)", asFUNCTION(CMaterialHandleTagConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Material", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CMaterialDestructor), asCALL_CDECL_OBJLAST); assert(r >= 0);

	r = engine->RegisterObjectType("Renderable", sizeof(CRenderable), asOBJ_VALUE | asGetTypeTraits<CRenderable>() | asOBJ_APP_CLASS_MORE_CONSTRUCTORS); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Renderable", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CRenderableDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Renderable", asBEHAVE_CONSTRUCT, "void f(uint, const Math::Transform@+)", asFUNCTION(CRenderableConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Renderable", asBEHAVE_CONSTRUCT, "void f(Renderable &in)", asFUNCTION(CRenderableCopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Renderable", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CRenderableDestructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "Renderable &opAssign(Renderable &in)", asMETHODPR(CRenderable, operator=, (CRenderable&), CRenderable&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "Material& get_material() property", asMETHOD(CRenderable, Get_material), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "void set_material(Material &in) property", asMETHOD(CRenderable, Set_material), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "Mesh& get_mesh() property", asMETHOD(CRenderable, Get_mesh), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Renderable", "void set_mesh(Mesh &in) property", asMETHOD(CRenderable, Set_mesh), asCALL_THISCALL); assert(r >= 0);
}
