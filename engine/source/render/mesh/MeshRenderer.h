#pragma once

#include "math/Math.h"
#include "Mesh.h"
#include "render/Renderable.h"

struct RenderContext;

class MeshRenderer {
public:
	static void DrawMesh(RenderContext& ctx, const Renderable& obj);
	static void DrawGeometryOnly(RenderContext& ctx, const Renderable& obj);

public:
	__declspec(align(16))
	struct CBVS {
		Math::Matrix worldTransform;
		Math::Matrix viewProj;
	};
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
		};
		static constexpr int NR_POINT_LIGHTS = 4;
		PointLight pointLights[NR_POINT_LIGHTS];
		Math::Vector4 uCameraPos;
		Math::Color uAmbientLight;
		int spotLightsNum = 0;
		int isTextureSet = 0;
		uint32_t entityId = 0;
	};
	__declspec(align(16))
	struct CBPSGeom {
		Math::Color ambientColor;
		int isTextureSet = 0;
		uint32_t entityId = 0;
	};
};
