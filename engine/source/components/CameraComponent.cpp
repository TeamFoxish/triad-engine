#include "CameraComponent.h"

#include "render/Renderer.h"
#include "game/Game.h"

#include "render/RenderSystem.h"
#include "render/RenderStorage.h"

#include "runtime/EngineRuntime.h" // TEMP

CameraComponent::CameraComponent(Game* game, const Camera::Params& params, Compositer* parent)
	: Component(game, parent)
	, handle(RenderStorage::Instance().cameras.Add(params, parent->GetTransformHandle()))
{
}

CameraComponent::~CameraComponent()
{
	RenderStorage::Instance().cameras.Remove(handle);
}
