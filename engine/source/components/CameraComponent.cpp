#include "CameraComponent.h"

#include "render/Renderer.h"
#include "game/Game.h"

#include "render/RenderSystem.h"

#include "runtime/EngineRuntime.h" // TEMP

CameraComponent::CameraComponent(Game* game, const CameraParams& params, Compositer* parent)
	: Component(game, parent)
	, projMatr(params.MakeProjectionMatrix())
	, cachedParams(params.Clone())
{
	game->SetActiveCamera(this);
	gViewportResized.AddLambda([this](int width, int height) {
		cachedParams->UpdateViewport(width, height);
		projMatr = cachedParams->MakeProjectionMatrix();
	});
}

CameraComponent::~CameraComponent()
{
	gViewportResized.Remove(viewportUpdateHandle);
}

void CameraComponent::SetViewMatrix(const Math::Matrix& view)
{
	gRenderSys->GetRenderer()->SetViewMatrix(view);
}

Math::Matrix CameraParamsPerspective::MakeProjectionMatrix() const
{
	return Math::Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, nearPlane, farPlane);
}

Math::Matrix CameraParamsOrthographic::MakeProjectionMatrix() const
{
	return Math::Matrix::CreateOrthographic(width, height, nearPlane, farPlane);
}
