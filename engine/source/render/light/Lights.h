#pragma once

#include "game/Component.h"
#include "math/Math.h"

#include "render/mesh/MeshRenderer.h"
#include "shared/TransformStorage.h"


class Renderer;


class Light {
public:
	__declspec(align(16))
	struct CBPS {
		struct DirectionalLight {
			Math::Vector4 mDirection;
			Math::Color mDiffuseColor;
			Math::Color mSpecColor;
		} dirLight;
		struct PointLight {
			Math::Vector4 position;
			Math::Vector4 diffuse;
			Math::Vector4 specular;
			float constant = 0.0f;
			float linear = 0.0f;
			float quadratic = 0.0f;
			float _dummy = 0.0f;
		} pointLight;
	};

	Light() = default;
	Light(Renderer* renderer, Compositer* parent);
	virtual ~Light();

	Math::Vector3 GetPos() const;

	const Math::Color& GetColor() const { return color; }
	void SetColor(const Math::Color& col) { color = col; }

	virtual void UpdateBuffer(CBPS& buffer, TransformStorage::Handle transform) const = 0;

protected:
	Renderer* GetRenderer() const;

protected:
	Compositer* parentRef;
	Math::Color color = {1.0f, 1.0f, 1.0f};
	Math::Color specularColor = {1.0f, 1.0f, 1.0f};
};


class DirectionalLight : public Light {
public:
	DirectionalLight() = default;
	DirectionalLight(Renderer* renderer, Compositer* parent);

	void UpdateBuffer(CBPS& buffer, TransformStorage::Handle transform) const override;
};


class PointLight : public Light {
public:
	enum AttenuationType {
		Constant = 0,
		Linear,
		Quadratic
	};

	PointLight() = default;
	PointLight(Renderer* renderer, Compositer* parent);

	void UpdateBuffer(CBPS& buffer, TransformStorage::Handle transform) const override;

	float GetIntensity() const { return intensity; }
	void SetIntensity(float _intensity) { intensity = _intensity; }

	float GetRadius() const { return radius; }
	void SetRadius(float _radius) { radius = _radius; }

protected:
	AttenuationType attenuation = Quadratic;
	float intensity = 10.0f;
	float radius = 25.0f;
};
