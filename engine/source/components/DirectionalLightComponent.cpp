#include "DirectionalLightComponent.h"

#include "core/Game.h"

DirectionalLightComponent::DirectionalLightComponent(Game* game, Compositer* parent)
	: Component(game, parent)
	, dirLight(game->GetRenderer(), parent)
{
}
