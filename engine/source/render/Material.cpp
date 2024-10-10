#include "Material.h"

#include "Renderer.h"
#include "Shader.h"

Material::Material(std::shared_ptr<Shader> _shader) 
	: shader(_shader)
{
}

// TODO: replace Renderer param with context ptr?
void Material::Use(Renderer* renderer)
{
	shader->Activate(renderer->GetDeviceContext());
}
