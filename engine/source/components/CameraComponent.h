#pragma once

#include "core/Component.h"
#include "core/Math.h"


struct CameraParams {
	virtual Math::Matrix MakeProjectionMatrix() const = 0;

	float nearPlane = 0.5f;
	float farPlane = 1000.0f;
};

struct CameraParamsPerspective : public CameraParams {
	Math::Matrix MakeProjectionMatrix() const override;

	float fov = 1.57f;
	float aspectRatio = 1280.0f / 720.0f;
};

struct CameraParamsOrthographic : public CameraParams {
	Math::Matrix MakeProjectionMatrix() const override;

	float width = 1280;
	float height = 720;
};

class CameraComponent : public Component {
public:
	CameraComponent(Game* game, const CameraParams& params, Compositer* parent = nullptr);

	virtual Math::Vector3 GetForwardVector(Compositer* parent) const { return Math::Vector3::Zero; }

	void SetProjectionMatrix(const Math::Matrix& matr) { projMatr = matr; }
	const Math::Matrix& GetProjectionMatrix() const { return projMatr; }

protected:
	void SetViewMatrix(const Math::Matrix& view);

	Math::Matrix projMatr;
};
