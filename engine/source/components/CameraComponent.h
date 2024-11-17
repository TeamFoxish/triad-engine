#pragma once

#include "game/Component.h"
#include "math/Math.h"
#include "misc/Delegates.h"

#include <memory>

struct CameraParams {
	enum ProjectionType {
		None = 0,
		Perspective,
		Orthographic
	};

	virtual Math::Matrix MakeProjectionMatrix() const = 0;

	// TEMP
	virtual void UpdateViewport(int width, int height) = 0;
	virtual CameraParams* Clone() const = 0;

	ProjectionType type;
	float nearPlane = 0.5f;
	float farPlane = 1000.0f;
};

struct CameraParamsPerspective : public CameraParams {
	Math::Matrix MakeProjectionMatrix() const override;
	void UpdateViewport(int width, int height) override { aspectRatio = (float)width / height; }
	CameraParams* Clone() const override { return new CameraParamsPerspective(*this); }

	float fov = 1.57f;
	float aspectRatio = 1280.0f / 720.0f;
};

struct CameraParamsOrthographic : public CameraParams {
	Math::Matrix MakeProjectionMatrix() const override;
	void UpdateViewport(int _width, int _height) override 
	{
		width = (float)_width;
		height = (float)_height;
	}
	CameraParams* Clone() const override { return new CameraParamsOrthographic(*this); }

	float width = 1280;
	float height = 720;
};

class CameraComponent : public Component {
public:
	CameraComponent(Game* game, const CameraParams& params, Compositer* parent = nullptr);
	~CameraComponent();

	virtual Math::Vector3 GetForwardVector(Compositer* parent) const { return Math::Vector3::Zero; }

	void SetProjectionMatrix(const Math::Matrix& matr) { projMatr = matr; }
	const Math::Matrix& GetProjectionMatrix() const { return projMatr; }

protected:
	void SetViewMatrix(const Math::Matrix& view);

	Math::Matrix projMatr;

	std::unique_ptr<CameraParams> cachedParams; // TEMP

	DelegateHandle viewportUpdateHandle;
};
