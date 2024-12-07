#pragma once

#include "fg/FrameGraphResource.hpp"
#include "render/FrameGraphResources.h"
#include "render/Shader.h"

class FrameGraph;
class FrameGraphBlackboard;

class LightVolumesPass {
public:
	struct LightVolumesPassData {
		FrameGraphResource depthStencilCopy;
	};

	LightVolumesPass(RenderContext& ctx);

	void AddLightVolumesPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard);

protected:
	struct ID3D11DepthStencilState* dsState1 = nullptr;
	struct ID3D11RasterizerState* rastState1 = nullptr;
	struct ID3D11DepthStencilState* dsState2 = nullptr;
	struct ID3D11RasterizerState* rastState2 = nullptr;
	struct ID3D11BlendState* blendState = nullptr;
	struct ID3D11Buffer* inversedProjBuf = nullptr;

	std::shared_ptr<Shader> pureGeomShader;
	std::shared_ptr<Shader> lightVolumesShader;

	float clearColor[4]{0.1f, 0.1f, 0.1f, 1.0f};
};
