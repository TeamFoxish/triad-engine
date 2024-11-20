#include "RenderResources.h"

bool InitRenderResources()
{
	RenderResources::instance = std::make_unique<RenderResources>();
	return true;
}

void TermRenderResources()
{
	RenderResources::instance.reset();
}
