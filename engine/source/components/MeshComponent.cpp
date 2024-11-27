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
	: DrawComponent(game, parent)
	, parent(parent)
{
	// TEMP
	SetMaterial(RenderResources::Instance().materials.Get(ToStrid("res://materials/default_mesh.material")));
}

void MeshComponent::Draw(Renderer* renderer)
{
	auto shader = renderer->GetUtils()->GetAdvMeshShader(renderer, sizeof(MeshRenderer::CBVS), sizeof(MeshRenderer::CBPS));
	auto context = renderer->GetDeviceContext();
	shader->Activate(context);
	auto material = GetMaterial().lock();
	//material->Use(renderer->GetContext());
	auto window = renderer->GetWindow();
	auto cbVS = MeshRenderer::CBVS{};
	cbVS.worldTransform = parent->GetWorldTransform().Transpose();
	cbVS.viewProj = renderer->GetViewMatrix();
	auto cbPS = MeshRenderer::CBPS{};
	cbPS.uAmbientLight = Math::Color{0.2f, 0.2f, 0.2f};
	ThirdPersonCamera* cam = static_cast<ThirdPersonCamera*>(GetGame()->GetActiveCamera());
	cbPS.uCameraPos = Math::Vector4(cam->GetCameraPos());
	renderer->PopulateLightsBuffer(cbPS); // TODO: TEMP E2
	cbPS.isTextureSet = material->HasBindedTextures() ? 1 : 0;
	cbPS.entityId = GetId();
	shader->SetCBVS(context, 0, &cbVS);
	shader->SetCBPS(context, 0, &cbPS);
	
	DrawComponent::Draw(renderer);
}

MeshComponent* MeshComponent::Build(Mesh::PTR mesh, CompositeComponent* parent)
{
	if (!mesh || mesh->GetRoot().geoms.empty()) {
		return nullptr;
	}
	return BuildMeshNode(mesh->GetRoot(), parent);
}

MeshComponent* MeshComponent::BuildMeshNode(const Mesh::MeshNode& node, CompositeComponent* parent)
{
	parent->SetPosition(node.pos);
	parent->SetRotation(node.rot);
	parent->SetScale(node.scale);
	Renderer* renderer = gRenderSys->GetRenderer();
	MeshComponent* tempRes = nullptr;
	for (const GeometryData::PTR& geom : node.geoms) {
		MeshComponent* meshComp = new MeshComponent(parent->GetGame(), parent);
		meshComp->SetGeometry(geom);
		//auto mat = std::make_shared<DefaultMeshMaterial>(parent->GetGame()->GetRenderer()->GetUtils()->GetAdvMeshShader(parent->GetGame()->GetRenderer(), sizeof(DefaultMeshMaterial::CBVS), sizeof(DefaultMeshMaterial::CBPS)));
		//meshComp->SetMaterial(mat);
		if (!tempRes) {
			tempRes = meshComp;
		}
	}
	for (const Mesh::MeshNode& child : node.children) {
		CompositeComponent* childComp = new CompositeComponent(parent->GetGame(), parent);
		BuildMeshNode(child, childComp);
	}

	return tempRes;
}
