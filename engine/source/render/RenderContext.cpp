#include "RenderContext.h"

void RenderContext::ResetFrame()
{
	viewportResized = false;
	backbuffResized = false;
}

void RenderContext::ResizeViewport(const Math::Viewport& _viewport)
{
	if (viewport == _viewport) {
		return;
	}
	viewport = _viewport;
	viewportResized = true;
}
