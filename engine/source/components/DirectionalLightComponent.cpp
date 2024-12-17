#include "DirectionalLightComponent.h"

#include "game/Game.h"

#include "render/RenderSystem.h"

DirectionalLightComponent::DirectionalLightComponent(Game* game, Compositer* parent)
	: Component(game, parent)
{
	LightsStorage::StorageImpl<DirectionalLight>::LightSource lightSrc;
	lightSrc.light = std::make_unique<DirectionalLight>();
	lightSrc.transform = parent->GetTransformHandle();
	handle = LightsStorage::Instance().dirLights.Add(std::move(lightSrc));
}

DirectionalLightComponent::~DirectionalLightComponent()
{
	LightsStorage::Instance().dirLights.Remove(handle);
}
