#include "Lights.h"

#include "render/Renderer.h"
#include "game/Game.h"
#include "game/Component.h"
#include "game/Component.h"

#include "render/RenderSystem.h"
#include "shared/SharedStorage.h"

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
	return gRenderSys->GetRenderer();
}

DirectionalLight::DirectionalLight(Renderer* renderer, Compositer* parent)
	: Light(renderer, parent)
{
}

void DirectionalLight::UpdateBuffer(CBPS& buffer, TransformStorage::Handle transform) const
{
	buffer.dirLight.mDiffuseColor = color;
	buffer.dirLight.mSpecColor = specularColor;

	const Math::Transform& lightTrs = SharedStorage::Instance().transforms.AccessRead(transform);
	Math::Vector3 forward = Math::Vector3::UnitX;
	buffer.dirLight.mDirection = Math::Vector4(Math::Vector3::Transform(forward, lightTrs.GetRotation()));
}

PointLight::PointLight(Renderer* renderer, Compositer* parent)
	: Light(renderer, parent)
{
}

void PointLight::UpdateBuffer(CBPS& buffer, TransformStorage::Handle transform) const
{
	CBPS::PointLight& pointL = buffer.pointLight;
	const Math::Transform& lightTrs = SharedStorage::Instance().transforms.AccessRead(transform);
	pointL.position = Math::Vector4(lightTrs.GetPosition());
	pointL.diffuse = color * intensity;
	pointL.specular = specularColor;
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
}
