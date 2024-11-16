#include "RenderContext.h"

void RenderContext::ResizeViewport(const Math::Viewport& _viewport)
{
	if (viewport == _viewport) {
		return;
	}
	viewport = _viewport;
	viewportResized = true;
}
