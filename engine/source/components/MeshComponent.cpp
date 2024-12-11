#include "MeshComponent.h"

#include "game/Game.h"
#include "render/Renderer.h"
#include "render/RenderUtils.h"
#include "render/RenderResources.h"
#include "render/Shader.h"
#include "render/GeometryData.h"
#include "render/mesh/MeshRenderer.h"
#include "CompositeComponent.h"
#include "components/ThirdPersonCamera.h"

#include "render/RenderSystem.h"


MeshComponent::MeshComponent(Game* game, Compositer* parent)
	: Component(game, parent)
	, parent(parent)
	, renderObj(RenderableStorage::Instance().Add(GetId(), parent->GetTransformHandle()))
{
	// TEMP
	SetMaterial(RenderResources::Instance().materials.Get(ToStrid("res://materials/default_mesh.material")));
}

MeshComponent::~MeshComponent()
{
	RenderableStorage::Instance().Remove(renderObj);
}
