#pragma once

#include "render/DrawComponent.h"
#include "render/Renderable.h"
#include "math/Math.h"

class CompositeComponent;

class MeshComponent : public DrawComponent {
public:
	MeshComponent(Game* game, Compositer* parent);

	void Draw(Renderer* renderer) override;

	void SetMesh(const Mesh::PTR _mesh) { renderObj.mesh = _mesh; }

	std::weak_ptr<Material> GetMaterial() const { return renderObj.material; }
	void SetMaterial(const std::shared_ptr<Material> _material) { renderObj.material = _material; }

public:
	float boundingSphereRadius = 0.0f;

protected:
	Compositer* parent;
	Renderable renderObj;
};
