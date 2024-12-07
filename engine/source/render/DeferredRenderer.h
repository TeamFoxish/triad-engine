#pragma once

#include "passes/GBufferPass.h"
#include "passes/LightVolumesPass.h"
#include "passes/DeferredLightingPass.h"

class DeferredRenderer {
public:
	DeferredRenderer(RenderContext& ctx);
	~DeferredRenderer() = default;

	void Draw(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard);

protected:
	GBufferPass gBufferPass;
	LightVolumesPass lightVolumesPass;
	DeferredLightingPass deferredLightPass;
};
