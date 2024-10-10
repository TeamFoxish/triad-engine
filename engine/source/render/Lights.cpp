#include "Lights.h"

#include "Renderer.h"
#include "core/Game.h"
#include "core/Component.h"
#include "core/Compositer.h"

Light::Light(Renderer* renderer, Compositer* parent)
	: parentRef(parent)
{
	renderer->AddLight(this);
}

Light::~Light() 
{
	GetRenderer()->RemoveLight(this);
}

Math::Vector3 Light::GetPos() const
{
	return parentRef->GetPosition();
}

Renderer* Light::GetRenderer() const
{
	return parentRef->GetGamePtr()->GetRenderer();
}

DirectionalLight::DirectionalLight(Renderer* renderer, Compositer* parent)
	: Light(renderer, parent)
{
}

void DirectionalLight::UpdateBuffer(DefaultMeshMaterial::CBPS& buffer) const
{
	buffer.dirLight.mDiffuseColor = color;
	buffer.dirLight.mSpecColor = specularColor;
	buffer.dirLight.mDirection = Math::Vector4(parentRef->GetForward());
}

PointLight::PointLight(Renderer* renderer, Compositer* parent)
	: Light(renderer, parent)
{
}

void PointLight::UpdateBuffer(DefaultMeshMaterial::CBPS& buffer) const
{
	if (buffer.spotLightsNum >= DefaultMeshMaterial::NR_POINT_LIGHTS) {
		return;
	}

	DefaultMeshMaterial::CBPS::PointLight& pointL = buffer.pointLights[buffer.spotLightsNum];
	pointL.diffuse = color * intensity;
	pointL.specular = specularColor;
	pointL.position = Math::Vector4(parentRef->GetPosition());
	switch (attenuation) {
	case Constant:
		pointL.constant = 1.0f;
		break;
	case Linear:
		pointL.linear = 1.0f;
		break;
	case Quadratic:
		pointL.quadratic = 1.0f;
		break;
	default:
		assert(false);
	}
	buffer.spotLightsNum++;
}
