#include "CameraComponent.h"

#include "render/Renderer.h"
#include "core/Game.h"

#include "render/RenderSystem.h"

CameraComponent::CameraComponent(Game* game, const CameraParams& params, Compositer* parent)
	: Component(game, parent)
	, projMatr(params.MakeProjectionMatrix())
{
	game->SetActiveCamera(this);
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
