#include "Game.h"

#include <windows.h>
#include <WinUser.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include <chrono>

#include "Component.h"
#include "os/Window.h"
#include "render/Renderer.h"
#include "render/RenderUtils.h"
#include "render/Shader.h"
#include "render/MeshLoader.h"
#include "render/Lights.h"

#include "components/CompositeComponent.h"
#include "components/ThirdPersonCamera.h"
#include "components/MeshComponent.h"
#include "components/PlayerBall.h"
#include "components/DirectionalLightComponent.h"
#include "components/PointLightComponent.h"
#include "components/OrbiterComponent.h"

#include "input/InputDevice.h"

#ifdef _WIN32
#include "os/wnd.h"
#endif


bool Game::Initialize(const std::string name, int windowWidth, int windowHeight)
{
	window = osCreateWindow(name, windowWidth, windowHeight);
	if (!window) {
		// TODO: log error message
		return false;
	}

	renderer = std::make_unique<Renderer>();
	if (!renderer->Initialize(window)) {
		// TODO: log error message
		return false;
	}
	float clearColor[] = {0.1f, 0.1f, 0.1f, 0.1f};
	renderer->SetClearColor(clearColor);

	globalInputDevice = new InputDevice(this);

	srand(std::time(0));

	LoadData();

	prevTime = std::chrono::steady_clock::now();

	return true;
}

void Game::ProcessInput()
{
	// Handle the windows messages.
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// If windows signals to end the application then exit out.
	if (msg.message == WM_QUIT) {
		isRunning = false;
		return;
	}

	for (Component* comp : components) {
		comp->ProceedInput(globalInputDevice);
	}
}

void Game::Shutdown()
{
	delete(globalInputDevice);
}

void Game::RunLoop()
{
	while (isRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::Restart()
{
	UnloadData();
	LoadData();
}

void Game::LoadData()
{
	auto rndPos = [](float extendX, float extendY) {
		auto rndFloat = []() { return (float)rand() / RAND_MAX; };
		const float x = extendX * 2;
		const float y = extendY * 2;
		return Math::Vector3(rndFloat() * x - extendX, rndFloat() * y - extendY, 0.0f);
	};
	auto rndColor = []() {
		auto rndFloat = []() { return (float)rand() / RAND_MAX; };
		return Math::Color(rndFloat(), rndFloat(), rndFloat());
	};
	
	{
		CompositeComponent* dirLightSocket = new CompositeComponent(this);
		DirectionalLightComponent* dirLight = new DirectionalLightComponent(this, dirLightSocket);
		dirLight->GetLightSource().SetColor(Math::Color(0.75, 0.75f, 0.75f));
		dirLightSocket->SetRotation(Math::Quaternion::CreateFromYawPitchRoll(Math::Pi / 4.0f, -Math::Pi / 4.0f, 0.0f));
	}

	{
		CompositeComponent* pointLightC = new CompositeComponent(this);
		MeshComponent* mesh = new MeshComponent(this, pointLightC);
		mesh->SetGeometry(GetRenderer()->GetUtils()->GetSphereGeom(renderer.get()));
		mesh->SetColor(Math::Color{1.0f, 0.0f, 0.0f});
		PointLightComponent* pointLight = new PointLightComponent(this, pointLightC);
		pointLight->GetLightSource().SetColor(Math::Color{1.0f, 0.0f, 0.0f});
		pointLight->GetLightSource().SetIntensity(25.0f);
		OrbiterComponent* orbiter = new OrbiterComponent(this, pointLightC, Math::Vector3{0.0f, 0.0f, 2.5f}, 10.0f, -1.0f);
		orbiter->SetMoveSpeed(1.0f);
		pointLightC->SetScale(Math::Vector3{0.2f});
	}
	{
		CompositeComponent* pointLightC = new CompositeComponent(this);
		MeshComponent* mesh = new MeshComponent(this, pointLightC);
		mesh->SetGeometry(GetRenderer()->GetUtils()->GetSphereGeom(renderer.get()));
		mesh->SetColor(Math::Color{0.0f, 1.0f, 0.0f});
		PointLightComponent* pointLight = new PointLightComponent(this, pointLightC);
		pointLight->GetLightSource().SetColor(Math::Color{0.0f, 1.0f, 0.0f});
		pointLight->GetLightSource().SetIntensity(25.0f);
		OrbiterComponent* orbiter = new OrbiterComponent(this, pointLightC, Math::Vector3{0.0f, 0.0f, 2.5f}, 10.0f, -1.0f);
		orbiter->SetMoveSpeed(1.0f);
		pointLightC->SetScale(Math::Vector3{0.2f});
	}
	{
		CompositeComponent* pointLightC = new CompositeComponent(this);
		MeshComponent* mesh = new MeshComponent(this, pointLightC);
		mesh->SetGeometry(GetRenderer()->GetUtils()->GetSphereGeom(renderer.get()));
		mesh->SetColor(Math::Color{0.0f, 0.0f, 1.0f});
		PointLightComponent* pointLight = new PointLightComponent(this, pointLightC);
		pointLight->GetLightSource().SetColor(Math::Color{0.0f, 0.0f, 1.0f});
		pointLight->GetLightSource().SetIntensity(25.0f);
		OrbiterComponent* orbiter = new OrbiterComponent(this, pointLightC, Math::Vector3{0.0f, 0.0f, 2.5f}, 10.0f, -1.0f);
		orbiter->SetMoveSpeed(1.0f);
		pointLightC->SetScale(Math::Vector3{0.2f});
	}

	player = new PlayerBall(this);
	player->Initialize();

	/*CompositeComponent* temp = new CompositeComponent(this);
	MeshComponent* tempMesh = new MeshComponent(this, temp);
	tempMesh->SetShader(GetRenderer()->GetUtils()->GetMeshShader(renderer.get()));
	tempMesh->SetGeometry(GetRenderer()->GetUtils()->GetCubeGeom(renderer.get()));*/

	for (int i = 0; i < 25; ++i) {
		CompositeComponent* flopa = new CompositeComponent(this);
		const Mesh::PTR& mesh = renderer->GetMesh("assets/flop.fbx");
		MeshComponent* rootMesh = MeshComponent::Build(mesh, flopa);
		if (rootMesh) {
			Texture tex(0, L"assets/flopTex.png", renderer.get());
			rootMesh->SetTexture(tex);
			// TODO: i feel really bad about this
			flopa->boundingSphereRadius = rootMesh->boundingSphereRadius;
		}
		flopa->SetPosition(rndPos(20, 20));
		flopa->SetRotation(Math::Quaternion::CreateFromYawPitchRoll(0.0f, Math::Pi / 2, 0.0f));
		flopa->SetScale(Math::Vector3{ 0.005f });
		flopa->Initialize();
		player->sceneObjects.push_back(flopa);
	}
	for (int i = 0; i < 25; ++i) {
		CompositeComponent* flopa = new CompositeComponent(this);
		const Mesh::PTR& mesh = renderer->GetMesh("assets/cheese.fbx");
		MeshComponent* rootMesh = MeshComponent::Build(mesh, flopa);
		if (rootMesh) {
			Texture tex(0, L"assets/cheeseTex.jpg", renderer.get());
			rootMesh->SetTexture(tex);
			// TODO: i feel really bad about this
			flopa->boundingSphereRadius = rootMesh->boundingSphereRadius;
		}
		flopa->SetPosition(rndPos(20, 20));
		//flopa->SetRotation(Math::Quaternion::CreateFromYawPitchRoll(Math::Pi / 2, 0.0f, 0.0f));
		//flopa->SetScale(Math::Vector3{ 1.0f });
		flopa->Initialize();
		player->sceneObjects.push_back(flopa);
	}

	/*{
		player = new CompositeComponent(this);
		CameraParamsPerspective perspective;
		perspective.aspectRatio = (float)window->GetWidth() / window->GetHeigth();
		camera = new ThirdPersonCamera(this, perspective, player);
		player->SetPosition(Math::Vector3{ 0.0f, 0.0f, 0.0f });
		player->Initialize();
	}
	{
		CompositeComponent* flopa = new CompositeComponent(this);
		MeshComponent* rootMesh = nullptr;
		MeshLoader::LoadMesh("assets/flop.fbx", flopa, &rootMesh);
		Texture tex(0, L"assets/flopTex.png", renderer.get());
		if (rootMesh) {
			rootMesh->SetTexture(tex);
		}
		flopa->SetScale(Math::Vector3{ 0.01f });
		flopa->SetRotation(Math::Quaternion::CreateFromYawPitchRoll(0.0f, Math::Pi / 2, 0.0f));
		flopa->Initialize();
	}*/
}

void Game::UnloadData()
{
	while (!components.empty())
	{
		delete components.back();
	}
}

void Game::UpdateGame()
{
	const auto curTime = std::chrono::steady_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - prevTime).count() / 1000000.0f;
	prevTime = curTime;
	totalTime += deltaTime;
	frameNum++;

	if (totalTime > 1.0f) {
		float fps = frameNum / totalTime;

		totalTime -= 1.0f;

#ifdef _WIN32
		WCHAR text[256];
		swprintf_s(text, TEXT("FPS: %f"), fps);
		SetWindowText(wndGetHWND(window), text);
#endif

		frameNum = 0;
	}

	isUpdatingComponents = true;
	for (auto comp : components)
	{
		comp->Update(deltaTime);
	}
	isUpdatingComponents = false;

	for (auto comp : pendingComponents)
	{
		components.emplace_back(comp);
	}
	pendingComponents.clear();
}

void Game::GenerateOutput()
{
	renderer->Draw();
}

CompositeComponent* Game::GetCameraHolder()
{
	return static_cast<CompositeComponent*>(player);
}

void Game::AddComponent(Component* comp)
{
#ifdef _DEBUG
	auto iter = std::find(pendingComponents.begin(), pendingComponents.end(), comp);
	assert(iter == pendingComponents.end());
	iter = std::find(components.begin(), components.end(), comp);
	assert(iter == components.end());
#endif

	if (isUpdatingComponents) {
		pendingComponents.push_back(comp);
		return;
	}
	components.push_back(comp);
}

void Game::RemoveComponent(Component* comp)
{
	auto iter = std::find(pendingComponents.begin(), pendingComponents.end(), comp);
	if (iter != pendingComponents.end())
	{
		std::iter_swap(iter, pendingComponents.end() - 1);
		pendingComponents.pop_back();
		return;
	}

	iter = std::find(components.begin(), components.end(), comp);
	if (iter != components.end())
	{
		std::iter_swap(iter, components.end() - 1);
		components.pop_back();
		return;
	}

	assert(false);
}
