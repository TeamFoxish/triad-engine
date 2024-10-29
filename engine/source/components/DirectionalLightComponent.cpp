#include "DirectionalLightComponent.h"

#include "core/Game.h"

#include "render/RenderSystem.h"

DirectionalLightComponent::DirectionalLightComponent(Game* game, Compositer* parent)
	: Component(game, parent)
	, dirLight(gRenderSys->GetRenderer(), parent)
{
}
