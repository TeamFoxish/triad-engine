#pragma once

#include "Shared.h"
#include "render/Renderable.h"
#include "render/RenderResources.h"
#include "resource/ResourceSystem.h"
#include "MathScriptBindings.h"

class asITypeInfo;

#define DECLARE_RESOURCE_HANDLE(TYPE, NAME, VAR)    \
	TYPE& Get_##NAME() { return NAME; }             \
	void Set_##NAME(const TYPE& handle)	            \
	{												\
		NAME = handle;								\
		NAME.Resolve(VAR);							\
	}												\
	TYPE NAME						                \

class CResourceHandle : public CNativeObject {
public:
	CResourceHandle() = default;
	CResourceHandle(ResTag _tag) 
		: tag(_tag) 
	{
	}

	bool IsValid() const { return tag != ResTag{}; }

	ResTag GetTag() const { return tag; }
	virtual Strid GetType() const { return Strid{}; }

	void ApplyOverrides(const YAML::Node& tag) override;
	YAML::Node Serialize() const override;

protected:
	ResTag tag;
};

class CResourceHandleDynamic : public CNativeObject {
public:
	CResourceHandleDynamic() = default;
	CResourceHandleDynamic(asITypeInfo* type)
		: objType(type)
	{
	}
	CResourceHandleDynamic(asITypeInfo* type, const std::string& _tag)
		: tagStr(_tag)
		, objType(type)
	{
	}

	bool IsValid() const { return !tagStr.empty() && Triad::Resource::IsTagDynamic(tagStr); }

	void ApplyOverrides(const YAML::Node& tag) override;

	YAML::Node Serialize() const override;

	void* Load() const;

protected:
	std::string tagStr;
	asITypeInfo* objType = nullptr;
};

class CMaterialHandle : public CResourceHandle {
public:
	CMaterialHandle() = default;
	CMaterialHandle(ResTag _tag)
		: CResourceHandle(_tag) 
	{
	}

	Strid GetType() const override { return ToStrid("material"); }

	void Set(const std::shared_ptr<Material>& material) { cached = material; }

	void Resolve(std::shared_ptr<Material>& out) {
#ifdef EDITOR
		if (!gResourceSys->IsResourceLoaded(tag)) {
			gResourceSys->LoadResource(tag);
		}
#endif // EDITOR
		out = cached = RenderResources::Instance().materials.GetRef(tag, cached); 
	}

private:
	// cache ptr to identify the resource even if it was changed by engine
	std::shared_ptr<Material> cached;
};

class CMeshHandle : public CResourceHandle {
public:
	CMeshHandle() = default;
	CMeshHandle(ResTag _tag)
		: CResourceHandle(_tag)
	{
	}

	Strid GetType() const override { return ToStrid("mesh"); }

	void Set(const std::shared_ptr<Mesh>& mesh) { cached = mesh; }

	void Resolve(std::shared_ptr<Mesh>& out) {
#ifdef EDITOR
		if (!gResourceSys->IsResourceLoaded(tag)) {
			gResourceSys->LoadResource(tag);
		}
#endif // EDITOR
		out = cached = RenderResources::Instance().meshes.GetRef(tag, cached); 
	}

private:
	// cache ptr to identify the resource even if it was changed by engine
	std::shared_ptr<Mesh> cached;
};

class CRenderable : public CNativeObject {
	friend void RegisterResourceHandles();

public:
	CRenderable() = default;
	CRenderable(int32_t entityId, const CTransformHandle& _transform)
		: renderObj(RenderableStorage::Instance().Add(entityId, _transform.GetHandle()))
	{
		// TEMP
		if (mesh.GetTag().hash_code() != 0) {
			Set_mesh(mesh);
		}
		if (material.GetTag().hash_code() != 0) {
			Set_material(material);
		}
	}

	CRenderable(CRenderable& other) 
	{
		renderObj = other.renderObj;
		other.renderObj = RenderableStorage::Handle{};
		mesh = other.mesh;
		material = other.material;
	}
	CRenderable(CRenderable&&) = delete;

	~CRenderable() 
	{
		if (renderObj.id_ < 0) {
			return;
		}
		RenderableStorage::Instance().Remove(renderObj);
	}

	void ApplyOverrides(const YAML::Node& overrides) override;

	YAML::Node Serialize() const override;

	Renderable& GetRenderObj() const { return RenderableStorage::Instance().Get(renderObj); }

	void SetIsStatic(bool isStatic) { GetRenderObj().params.isStatic = isStatic; }

	CRenderable& operator=(CRenderable& other) 
	{
		if (renderObj.id_ >= 0) {
			RenderableStorage::Instance().Remove(renderObj);
		}
		renderObj = other.renderObj;
		other.renderObj = RenderableStorage::Handle{};
		mesh = other.mesh;
		material = other.material;
		return *this;
	}

private:
	RenderableStorage::Handle renderObj;

	DECLARE_RESOURCE_HANDLE(CMeshHandle, mesh, GetRenderObj().mesh);
	DECLARE_RESOURCE_HANDLE(CMaterialHandle, material, GetRenderObj().material);
};
