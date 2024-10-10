#include "PointLightComponent.h"

#include "core/Game.h"

PointLightComponent::PointLightComponent(Game* game, Compositer* parent)
	: Component(game, parent)
	, pointLight(game->GetRenderer(), parent)
{
}
