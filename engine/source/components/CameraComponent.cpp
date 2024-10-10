#include "CameraComponent.h"

#include "render/Renderer.h"
#include "core/Game.h"

CameraComponent::CameraComponent(Game* game, const CameraParams& params, Compositer* parent)
	: Component(game, parent)
	, projMatr(params.MakeProjectionMatrix())
{
	game->SetActiveCamera(this);
}

void CameraComponent::SetViewMatrix(const Math::Matrix& view)
{
	GetGame()->GetRenderer()->SetViewMatrix(view);
}

Math::Matrix CameraParamsPerspective::MakeProjectionMatrix() const
{
	return Math::Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, nearPlane, farPlane);
}

Math::Matrix CameraParamsOrthographic::MakeProjectionMatrix() const
{
	return Math::Matrix::CreateOrthographic(width, height, nearPlane, farPlane);
}
