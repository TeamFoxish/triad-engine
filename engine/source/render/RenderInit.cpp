#include "RenderInit.h"

#include "RenderSystem.h"
#include "Renderer.h" // any possible way to evade such include?

bool InitRender(RuntimeIface* runtime)
{
	gRenderSys = std::make_unique<RenderSystem>();
	return gRenderSys->Init(runtime);
}

void TermRender(RuntimeIface* runtime)
{
	gRenderSys->Term();
	gRenderSys = nullptr;
}
