#pragma once

#include "math/Transform.h"

class Camera {
public:
    struct Params {
        enum ProjectionType {
            Perspective,
            Orthographic
        } type = Perspective;
        float nearPlane = 0.5f;
        float farPlane = 1000.0f;
        float width = 1280.0f;
        float height = 720.0f;
        float fov = 90.0f;
    };

    Camera(const Params& params, const Math::Transform& transform);

    const Math::Matrix& GetProjectionMatrix() const { return projMatr; }
    const Math::Matrix& GetViewMatrix() const { return viewMatr; }

    void UpdateProjection(const Params& params);
    void UpdateView(const Math::Transform& transform);

protected:
    Math::Matrix projMatr;
    Math::Matrix viewMatr;
};
