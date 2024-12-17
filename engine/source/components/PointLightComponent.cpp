#include "PointLightComponent.h"

#include "game/Game.h"

#include "render/RenderSystem.h"

PointLightComponent::PointLightComponent(Game* game, Compositer* parent)
	: Component(game, parent)
{
	LightsStorage::StorageImpl<PointLight>::LightSource lightSrc;
	lightSrc.light = std::make_unique<PointLight>();
	lightSrc.transform = parent->GetTransformHandle();
	handle = LightsStorage::Instance().pointLights.Add(std::move(lightSrc));
}

PointLightComponent::~PointLightComponent()
{
	LightsStorage::Instance().pointLights.Remove(handle);
}
