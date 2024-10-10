#pragma once

#include "render/Material.h"
#include "core/Math.h"

class DefaultMeshMaterial : public Material {
public:
	static constexpr int NR_POINT_LIGHTS = 4;

	struct CBVS {
		Math::Matrix worldTransform;
		Math::Matrix viewProj;
	};
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
		};
		PointLight pointLights[NR_POINT_LIGHTS];
		Math::Color color;
		Math::Vector4 uCameraPos;
		Math::Color uAmbientLight;
		float uSpecPower = 0.25f;
		float uShininess = 32.0f;
		int spotLightsNum = 0;
		int isTextureSet = 0;
	};

	DefaultMeshMaterial(std::shared_ptr<Shader> shader);

	void Use(Renderer* renderer) override;

	int GetCBVSSize() const override { return sizeof(CBVS); }
	int GetCBPSSize() const override { return sizeof(CBPS); }
};
