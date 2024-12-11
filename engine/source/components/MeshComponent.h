#pragma once

#include "game/Component.h"
#include "render/Renderable.h"
#include "math/Math.h"

class CompositeComponent;

class MeshComponent : public Component {
public:
	MeshComponent(Game* game, Compositer* parent);
	~MeshComponent();

	void SetMesh(const Mesh::PTR mesh) { GetRenderObj().mesh = mesh; }

	std::weak_ptr<Material> GetMaterial() const { return GetRenderObj().material; }
	void SetMaterial(const std::shared_ptr<Material> _material) { GetRenderObj().material = _material; }

protected:
	Renderable& GetRenderObj() { return RenderableStorage::Instance().Get(renderObj); }
	const Renderable& GetRenderObj() const { return RenderableStorage::Instance().Get(renderObj); }

public:
	float boundingSphereRadius = 0.0f;

protected:
	Compositer* parent;
	RenderableStorage::Handle renderObj;
};
