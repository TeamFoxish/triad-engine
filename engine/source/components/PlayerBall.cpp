#include "PlayerBall.h"

#include "game/Game.h"
#include "components/MeshComponent.h"
#include "components/ThirdPersonCamera.h"
#include "render/Renderer.h"
#include "render/mesh/MeshLoader.h"
#include "os/Window.h"
#include "components/PointLightComponent.h"
#include "render/Lights.h"

#include "render/RenderSystem.h"

#include <iostream>

PlayerBall::PlayerBall(Game* game)
	: CompositeComponent(game)
{
	boundingSphereRadius = 1.4241f;
}

void PlayerBall::Initialize(Compositer* parent)
{
	CameraParamsPerspective perspective;
	perspective.aspectRatio = gRenderSys->GetRenderer()->GetContext().viewport.width / gRenderSys->GetRenderer()->GetContext().viewport.height;
	camera = new ThirdPersonCamera(GetGame(), perspective, this);

	PointLightComponent* pointLight = new PointLightComponent(GetGame(), this);
	pointLight->GetLightSource().SetColor(Math::Color{1.0f, 0.9f, 0.5f});
	pointLight->GetLightSource().SetIntensity(10.0f);

	Renderer* renderer = gRenderSys->GetRenderer();
	meshSocket = new CompositeComponent(GetGame(), this);
	MeshComponent* mesh = meshComp = new MeshComponent(GetGame(), meshSocket);
	mesh->SetMesh(Mesh::CreateFromGeom(renderer->GetUtils()->GetSphereGeom(renderer)));

	CompositeComponent* tempC = new CompositeComponent(GetGame(), meshSocket);
	MeshComponent* cube = new MeshComponent(GetGame(), tempC);
	cube->SetMesh(Mesh::CreateFromGeom(renderer->GetUtils()->GetCubeGeom(renderer)));
	tempC->SetPosition(Math::Vector3{1.0f, 0.0f, 0.0f});
	tempC->SetScale(Math::Vector3{2.0f, 0.2f, 0.2f});

	attachSocket = new CompositeComponent(GetGame(), this);
	GetGame()->player = this;

	CompositeComponent::Initialize();
}

void PlayerBall::ProceedInput(InputDevice* inpDevice)
{
	const Math::Vector3 prevPos = GetPosition();
	CompositeComponent::ProceedInput(inpDevice);
	Math::Vector3 moveVec = GetPosition() - prevPos;
	if (moveVec.LengthSquared() < 0.001f) {
		return;
	}
	const float moveDist = moveVec.Length();
	moveVec.Normalize();
	const Math::Vector3 up = {0.0f, 0.0f, 1.0f};
	const Math::Vector3 right = Math::Vector3::Transform(moveVec, Math::Quaternion::CreateFromAxisAngle(up, -Math::Pi / 2));
	const Math::Vector3 rightLocal = Math::Vector3::Transform(right, meshSocket->GetRotation());
	const Math::Quaternion rot = Math::Quaternion::CreateFromAxisAngle(right, -moveDist);
	meshSocket->SetRotation(Math::Quaternion::Concatenate(rot, meshSocket->GetRotation()));
	attachSocket->SetRotation(meshSocket->GetRotation());

	std::vector<int> newChilds;
	float targetScale = meshSocket->GetScale().x;
	float targetCamDist = camera->GetRadius();
	for (int i = 0; i < sceneObjects.size(); ++i) {
		const float dist = (GetPosition() - sceneObjects[i]->GetPosition()).Length();
		if (dist > boundingSphereRadius * meshSocket->GetScale().x + sceneObjects[i]->boundingSphereRadius * sceneObjects[i]->GetScale().x) {
			continue;
		}
		newChilds.push_back(i);
		targetScale += 0.1f;
		targetCamDist += 0.1f;
	}
	for (int idx : newChilds) {
		GetGame()->RemoveComponent(sceneObjects[idx]);
		attachSocket->AddChild(sceneObjects[idx]);
	}
	for (int i = (int)newChilds.size() - 1; i >= 0; --i) {
		sceneObjects.erase(sceneObjects.begin() + newChilds[i]);
	}
	meshSocket->SetScale(Math::Vector3{targetScale});
	camera->SetRadius(targetCamDist);
}
