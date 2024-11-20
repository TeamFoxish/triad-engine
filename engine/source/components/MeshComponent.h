#pragma once

#include "render/DrawComponent.h"
#include "render/mesh/Mesh.h"
#include "render/material/Material.h"
#include "math/Math.h"

class CompositeComponent;

class MeshComponent : public DrawComponent {
public:
	MeshComponent(Game* game, Compositer* parent);

	void Draw(Renderer* renderer) override;

	void SetColor(const Math::Color& _color) { color = _color; }

	void SetMesh(const Mesh::PTR _mesh) { mesh = _mesh; }

	static MeshComponent* Build(Mesh::PTR mesh, CompositeComponent* parent);

protected:
	static MeshComponent* BuildMeshNode(const Mesh::MeshNode& node, CompositeComponent* parent);

public:
	float boundingSphereRadius = 0.0f;

protected:
	Compositer* parent;
	Mesh::PTR mesh;
	Math::Color color{ 1.0f, 1.0f, 1.0f, 1.0f };
};
