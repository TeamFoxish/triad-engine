#include "PointLightComponent.h"

#include "game/Game.h"

#include "render/RenderSystem.h"

PointLightComponent::PointLightComponent(Game* game, Compositer* parent)
	: Component(game, parent)
	, pointLight(gRenderSys->GetRenderer(), parent)
{
}
