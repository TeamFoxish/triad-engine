#include "PointLightComponent.h"

#include "core/Game.h"

#include "render/RenderSystem.h"

PointLightComponent::PointLightComponent(Game* game, Compositer* parent)
	: Component(game, parent)
	, pointLight(gRenderSys->GetRenderer(), parent)
{
}
