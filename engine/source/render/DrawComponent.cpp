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

void DrawComponent::Initialize(Compositer* parent)
{
	assert(material);
	assert(geom);
}

void DrawComponent::Draw(Renderer* renderer)
{
	ID3D11DeviceContext* context = renderer->GetDeviceContext();
	material->Use(renderer->GetContext());
	geom->Activate(context);
	context->DrawIndexed(geom->idxNum, 0, 0);
}
