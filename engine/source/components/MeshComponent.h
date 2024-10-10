#pragma once

#include "render/DrawComponent.h"
#include "render/Mesh.h"
#include "render/Texture.h"
#include "core/Math.h"

class CompositeComponent;

class MeshComponent : public DrawComponent {
public:
	MeshComponent(Game* game, Compositer* parent);

	void Draw(Renderer* renderer) override;

	void SetColor(const Math::Color& _color) { color = _color; }

	void SetMesh(const Mesh::PTR _mesh) { mesh = _mesh; }

	void SetTexture(const Texture& _tex) { tex = _tex; }

	static MeshComponent* Build(Mesh::PTR mesh, CompositeComponent* parent);

protected:
	static MeshComponent* BuildMeshNode(const Mesh::MeshNode& node, CompositeComponent* parent);

public:
	float boundingSphereRadius = 0.0f;

protected:
	Compositer* parent;
	Mesh::PTR mesh;
	Texture tex;
	Math::Color color{ 1.0f, 1.0f, 1.0f, 1.0f };
};
