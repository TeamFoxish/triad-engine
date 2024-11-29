#include "DrawComponent.h"

#include "Renderer.h"
#include "material/Material.h"
#include "GeometryData.h"

#include "render/RenderSystem.h"

#include <d3d11.h>
#include <cassert>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


DrawComponent::DrawComponent(Game* game, Compositer* compister)
	: Component(game, compister)
{
	gRenderSys->GetRenderer()->AddComponent(this);
}

DrawComponent::~DrawComponent()
{
	gRenderSys->GetRenderer()->RemoveComponent(this);
}
