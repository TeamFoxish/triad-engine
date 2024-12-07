#pragma once

#include "fg/FrameGraphResource.hpp"
#include "render/FrameGraphResources.h"
#include "render/Shader.h"

class FrameGraph;
class FrameGraphBlackboard;

class DeferredLightingPass {
public:
	struct DeferredLightingPassData {
		FrameGraphResource sceneColor;
	};

	DeferredLightingPass(RenderContext& ctx);

	void AddDeferredLightingPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard);

protected:
	void DrawFullScreenQuad(RenderContext& ctx);

protected:
	struct ID3D11Buffer* inversedProjBuf = nullptr;

	std::shared_ptr<Shader> shader;
};
