#include "Camera.h"

#include "logs/Logs.h"

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
    LOG_DEBUG("forward {} {} {}", Math::Vector3::Forward.x, Math::Vector3::Forward.y, Math::Vector3::Forward.z);
    LOG_DEBUG("right {} {} {}", Math::Vector3::Right.x, Math::Vector3::Right.y, Math::Vector3::Right.z);
    LOG_DEBUG("up {} {} {}", Math::Vector3::Up.x, Math::Vector3::Up.y, Math::Vector3::Up.z);
    const Math::Vector3 camPos = transform.GetPosition();
    const Math::Vector3 forward = Math::Vector3::Transform(Math::Vector3::Forward, transform.GetRotation());
    const Math::Vector3 left = Math::Vector3::Transform(Math::Vector3::Left, transform.GetRotation());
    const Math::Vector3 up = forward.Cross(left);

    //// Create look at matrix, set as view
    viewMatr = Math::Matrix::CreateLookAt(camPos, camPos + forward * 1.0f, up);
    //viewMatr = transform.GetMatrix();

    //const Math::Vector3 xAxis = left;
    //const Math::Vector3 yAxis = up;
    //const Math::Vector3 zAxis = forward;

    //Math::Matrix view2;
    //view2._11 = xAxis.x; // L
    //view2._21 = xAxis.y;
    //view2._31 = xAxis.z;

    //view2._12 = yAxis.x; // U
    //view2._22 = yAxis.y;
    //view2._32 = yAxis.z;

    //view2._13 = zAxis.x; // F
    //view2._23 = zAxis.y;
    //view2._33 = zAxis.z;

    ////view2._41 = camPos.x; // T
    ////view2._42 = camPos.y;
    ////view2._43 = camPos.z;

    //view2._41 = xAxis.Dot(-camPos); // T
    //view2._42 = yAxis.Dot(-camPos);
    //view2._43 = zAxis.Dot(-camPos);

    //view2._14 = 0.0f;
    //view2._24 = 0.0f;
    //view2._34 = 0.0f;
    //view2._44 = 1.0f;

    //viewMatr = view2;
}
