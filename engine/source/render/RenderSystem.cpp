#include "RenderSystem.h"

#include "Renderer.h"
#include "runtime/RuntimeIface.h"
#include "config/ConfigVar.h"

static ConfigVar<std::vector<float>> cfgRenderClearColor("/Engine/Render/ClearColor", { 0.0f, 0.0f, 0.0f, 0.0f });

std::unique_ptr<RenderSystem> gRenderSys = nullptr;

bool RenderSystem::Init(RuntimeIface* runtime)
{
	rendererImpl = std::make_unique<Renderer>();
	if (!rendererImpl->Initialize(runtime->GetWindow())) {
		return false;
	}
	rendererImpl->SetClearColor(cfgRenderClearColor.GetRef().data());
	return true;
}

void RenderSystem::Term()
{
	rendererImpl->Shutdown();
}

void RenderSystem::StartFrame()
{
	rendererImpl->Draw();
}

void RenderSystem::EndFrame()
{
	rendererImpl->EndFrame();
}