#pragma once

#include "math/Math.h"
#include "Mesh.h"
#include "render/Renderable.h"
#include "render/light/Lights.h"

struct RenderContext;

class MeshRenderer {
public:
	static void DrawGeometryOnly(RenderContext& ctx, const Renderable& obj);

public:
	__declspec(align(16))
	struct CBVS {
		Math::Matrix worldTransform;
		Math::Matrix viewProj;
	};
	__declspec(align(16))
	struct CBPS {
		Math::Vector4 uCameraPos;
		Math::Color uAmbientLight;
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
