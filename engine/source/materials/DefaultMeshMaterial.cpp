#include "DefaultMeshMaterial.h"

#include "core/Compositer.h"

DefaultMeshMaterial::DefaultMeshMaterial(std::shared_ptr<Shader> shader)
	: Material(shader)
{
}

void DefaultMeshMaterial::Use(Renderer* renderer)
{
	Material::Use(renderer);
}
