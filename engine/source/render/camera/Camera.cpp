#include "Camera.h"

Camera::Camera(const Params& params, const Math::Transform& transform)
{
    UpdateProjection(params);
    UpdateView(transform);
}

void Camera::UpdateProjection(const Params& params)
{
    switch (params.type) {
        case Params::Perspective:
        {
            const float fov = Math::DegToRad(params.fov);
            const float aspectRatio = params.width / params.height;
            projMatr = Math::Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, params.nearPlane, params.farPlane);
            break;
        }
        case Params::Orthographic:
            projMatr = Math::Matrix::CreateOrthographic(params.width, params.height, params.nearPlane, params.farPlane);
            break;
        default:
            assert(false);
    }
}

void Camera::UpdateView(const Math::Transform& transform)
{
    const Math::Vector3 camPos = transform.GetPosition();
    const Math::Vector3 forward = Math::Vector3::Transform(Math::Vector3::Forward, transform.GetRotation());
    const Math::Vector3 left = Math::Vector3::Transform(Math::Vector3::Left, transform.GetRotation());
    const Math::Vector3 up = forward.Cross(left);

    //// Create look at matrix, set as view
    viewMatr = Math::Matrix::CreateLookAt(camPos, camPos + forward * 1.0f, up);
}
