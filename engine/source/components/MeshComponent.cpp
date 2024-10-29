#include "MeshComponent.h"

#include "game/Game.h"
#include "render/Renderer.h"
#include "render/RenderUtils.h"
#include "render/Shader.h"
#include "render/GeometryData.h"
#include "CompositeComponent.h"
#include "materials/DefaultMeshMaterial.h"
#include "components/ThirdPersonCamera.h"

#include "render/RenderSystem.h"

MeshComponent::MeshComponent(Game* game, Compositer* parent)
	: DrawComponent(game, parent)
	, parent(parent)
{
	auto mat = std::make_shared<DefaultMeshMaterial>(gRenderSys->GetRenderer()->GetUtils()->GetAdvMeshShader(gRenderSys->GetRenderer(), sizeof(DefaultMeshMaterial::CBVS), sizeof(DefaultMeshMaterial::CBPS)));
	SetMaterial(mat);
}

void MeshComponent::Draw(Renderer* renderer)
{
	auto material = GetMaterial().lock();
	auto shader = material->GetShader();
	auto context = renderer->GetDeviceContext();
	auto window = renderer->GetWindow();
	auto cbVS = DefaultMeshMaterial::CBVS{};
	cbVS.worldTransform = parent->GetWorldTransform().Transpose();
	cbVS.viewProj = renderer->GetViewMatrix();
	auto cbPS = DefaultMeshMaterial::CBPS{};
	cbPS.color = color;
	cbPS.uAmbientLight = Math::Color{0.2f, 0.2f, 0.2f};
	ThirdPersonCamera* cam = static_cast<ThirdPersonCamera*>(GetGame()->GetActiveCamera());
	cbPS.uCameraPos = Math::Vector4(cam->GetCameraPos());
	cbPS.uSpecPower = 0.0f;
	/*cbPS.dirLight.mDiffuseColor = Math::Color{1.0f, 1.0f, 1.0f};
	cbPS.dirLight.mDirection = Math::Vector4{0.35f, 0.35f, -1.0f, 0.0f};
	cbPS.dirLight.mDirection.Normalize();
	cbPS.dirLight.mSpecColor = Math::Color{1.0f, 1.0f, 1.0f};*/
	renderer->PopulateLightsBuffer(cbPS); // TODO: TEMP E2
	cbPS.isTextureSet = 0;
	if (tex.IsValid()) {
		tex.Activate(context);
		cbPS.isTextureSet = 1;
	}
	else {
		//__debugbreak();
	}
	shader.SetCBVS(context, 0, &cbVS);
	shader.SetCBPS(context, 0, &cbPS);
	
	//shader.lock()->Activate(context);
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
