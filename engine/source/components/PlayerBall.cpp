#include "PlayerBall.h"

#include "game/Game.h"
#include "components/MeshComponent.h"
#include "components/ThirdPersonCamera.h"
#include "render/Renderer.h"
#include "render/MeshLoader.h"
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
	perspective.aspectRatio = (float)gRenderSys->GetRenderer()->GetWindow()->GetWidth() / gRenderSys->GetRenderer()->GetWindow()->GetHeigth();
	camera = new ThirdPersonCamera(GetGame(), perspective, this);

	PointLightComponent* pointLight = new PointLightComponent(GetGame(), this);
	pointLight->GetLightSource().SetColor(Math::Color{1.0f, 0.9f, 0.5f});
	pointLight->GetLightSource().SetIntensity(10.0f);

	Renderer* renderer = gRenderSys->GetRenderer();
	meshSocket = new CompositeComponent(GetGame(), this);
	MeshComponent* mesh = new MeshComponent(GetGame(), meshSocket);
	//mesh->SetShader(GetGame()->GetRenderer()->GetUtils()->GetMeshShader(renderer));
	mesh->SetGeometry(renderer->GetUtils()->GetSphereGeom(renderer));

	CompositeComponent* tempC = new CompositeComponent(GetGame(), meshSocket);
	MeshComponent* cube = new MeshComponent(GetGame(), tempC);
	//cube->SetShader(GetGame()->GetRenderer()->GetUtils()->GetMeshShader(renderer));
	cube->SetGeometry(renderer->GetUtils()->GetCubeGeom(renderer));
	cube->SetColor(Math::Color{1.0f, 0.0f, 0.0f});
	tempC->SetPosition(Math::Vector3{1.0f, 0.0f, 0.0f});
	tempC->SetScale(Math::Vector3{2.0f, 0.2f, 0.2f});

	attachSocket = new CompositeComponent(GetGame(), this);

	/*MeshComponent* rootMesh = nullptr;
	MeshLoader::LoadMesh("assets/flop.fbx", meshSocket, &rootMesh);
	Texture tex(0, L"assets/flopTex.png", renderer);
	if (rootMesh) {
		rootMesh->SetTexture(tex);
	}
	meshSocket->SetScale(Math::Vector3{ 0.01f });
	meshSocket->SetRotation(Math::Quaternion::CreateFromYawPitchRoll(0.0f, Math::Pi / 2, 0.0f));*/

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
		const Math::Matrix& local = attachSocket->GetWorldTransform(this);
		const Math::Matrix& other = sceneObjects[idx]->GetWorldTransform();
		const Math::Matrix res = other * local.Invert();
		sceneObjects[idx]->SetWorldTransform(res);
		attachSocket->AddChild({sceneObjects[idx]});
	}
	for (int i = (int)newChilds.size() - 1; i >= 0; --i) {
		sceneObjects.erase(sceneObjects.begin() + newChilds[i]);
	}
	meshSocket->SetScale(Math::Vector3{targetScale});
	camera->SetRadius(targetCamDist);
}
