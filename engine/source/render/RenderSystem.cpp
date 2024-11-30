#include "RenderSystem.h"

#include "Renderer.h"
#include "runtime/EngineRuntime.h"
#include "config/ConfigVar.h"

static ConfigVar<std::vector<float>> cfgRenderClearColor("/Engine/Render/ClearColor", { 0.0f, 0.0f, 0.0f, 0.0f });

std::unique_ptr<RenderSystem> gRenderSys = nullptr;

bool RenderSystem::Init(RuntimeIface* runtime)
{
	extern bool InitRenderResources();
	InitRenderResources();
	extern bool InitRenderableStorage();
	InitRenderableStorage();
	rendererImpl = std::make_unique<Renderer>();
	if (!rendererImpl->Initialize(runtime->GetWindow())) {
		return false;
	}
	rendererImpl->SetClearColor(cfgRenderClearColor.GetRef().data());
	viewportResizedHandle = gViewportResized.AddLambda([this](int width, int height) {
		RenderContext& ctx = rendererImpl->GetContext();
		Math::Viewport viewport;
		viewport.width = width;
		viewport.height = height;
		ctx.ResizeViewport(viewport);
	});
	runtime->GetWindow()->windowResized.AddLambda([this](int width, int height) {
		RenderContext& ctx = rendererImpl->GetContext();
		ctx.backbuffResized = true;
#ifndef EDITOR
		// viewport editor is resized by itself
		gViewportResized.Broadcast(width, height);
#endif
	});
	return true;
}

void RenderSystem::Term()
{
	gViewportResized.Remove(viewportResizedHandle);
	rendererImpl->Shutdown();
	extern void TermRenderableStorage();
	TermRenderableStorage();
	extern void TermRenderResources();
	TermRenderResources();
}

RenderContext& RenderSystem::GetContext() const
{
	return rendererImpl->GetContext();
}

void RenderSystem::StartFrame()
{
	rendererImpl->Draw();
}

void RenderSystem::EndFrame()
{
	rendererImpl->EndFrame();
}

uint32_t RenderSystem::GetEntityIdUnderCursor() const
{
	return rendererImpl->GetEntityIdUnderCursor();
}
