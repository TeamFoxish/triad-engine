#include "RenderContext.h"

void RenderContext::ResetFrame()
{
	viewportResized = false;
	backbuffResized = false;
}

void RenderContext::ClearState()
{
	context->ClearState();
	memset(activeRenderTargets, 0, sizeof(activeRenderTargets));
	activeRenderTargetNum = 0;
}

void RenderContext::ResizeViewport(const Math::Viewport& _viewport)
{
	if (viewport == _viewport) {
		return;
	}
	viewport = _viewport;
	viewportResized = true;
}

void RenderContext::TEMP_UpdateRenderTargetsNum()
{
	uint32_t& i = activeRenderTargetNum = 0;
	for (; i < MAX_ACTIVE_RENDER_TARGETS; ++i) {
		if (activeRenderTargets[i] == nullptr) {
			break;
		}
	}
}
