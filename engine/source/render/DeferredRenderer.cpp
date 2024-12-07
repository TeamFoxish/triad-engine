#include "DeferredRenderer.h"

DeferredRenderer::DeferredRenderer(RenderContext& ctx)
	: gBufferPass(ctx)
	, lightVolumesPass(ctx)
	, deferredLightPass(ctx)
{
}

void DeferredRenderer::Draw(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard)
{
	gBufferPass.AddGeometryPass(ctx, fg, bboard);
	lightVolumesPass.AddLightVolumesPass(ctx, fg, bboard);
	deferredLightPass.AddDeferredLightingPass(ctx, fg, bboard);
}
