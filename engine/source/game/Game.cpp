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
#include "render/mesh/MeshLoader.h"
#include "scene/Scene.h"

#include "components/CompositeComponent.h"
#include "components/ThirdPersonCamera.h"
#include "components/MeshComponent.h"
#include "components/PlayerBall.h"
#include "components/DirectionalLightComponent.h"
#include "components/PointLightComponent.h"
#include "components/OrbiterComponent.h"

#include "input/InputDevice.h"
#include "render/RenderSystem.h"
#include "render/Renderer.h"
#include "scripts/ScriptSystem.h"

#ifdef _WIN32
#include "os/wnd.h"
#endif

#ifdef EDITOR
#include "editor/ui_debug/UIDebug.h"
#include "components/EditorCamera.h"
#endif // EDITOR

Game::Game()
{
	scenes.push_back(std::make_unique<Scene>()); // TEMP
}

Game::~Game()
{
}

bool Game::Initialize()
{
	srand((uint32_t)std::time(0));

	//LoadData();

	prevTime = std::chrono::steady_clock::now();

	if (!gScriptSys->PostInitEvent()) {
		std::cout << "Failed to handle Init event in scripts !" << std::endl;
	}

#ifdef EDITOR
	Camera::Params params;
	RenderContext& ctx = gRenderSys->GetContext();
	params.width = gRenderSys->GetContext().viewport.width;
	params.height = gRenderSys->GetContext().viewport.height;
	editorCam = new EditorCamera(this, params);
	editorCam->Initialize();
#endif

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

#ifdef EDITOR
	if (!UIDebug::start_simulation)
	{
		gRenderSys->cameraManager.SetActiveCamera(editorCam->GetCameraHandle());
		if (!UIDebug::outliner.gizmo_focused)
		{
			editorCam->ProceedInput(globalInputDevice);
		}
		return;
	}
#endif // EDITOR

	for (const auto& pScene : scenes) {
		for (Component* comp : pScene->GetComponents()) {
			comp->ProceedInput(globalInputDevice);
		}
	}
}

void Game::Shutdown()
{
}

void Game::UpdateFrame()
{
	globalInputDevice->PrepareProceedInput();
	ProcessInput();
	UpdateGame();
	const Component::Id_T id = gRenderSys->GetEntityIdUnderCursor();
	if (id > 0) {
		Component* comp = scenes[0]->GetStorage().GetComponentById(gRenderSys->GetEntityIdUnderCursor());
		if (comp) {
			std::cout << "component under cursor: " << comp->GetName() << '\n';
		}
	}

	GenerateOutput();
}

void Game::Restart()
{
	UnloadData();
	LoadData();
}

// ToDo: remove or rewrite logic
void Game::LoadData()
{
#if 0
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
		mesh->SetGeometry(gRenderSys->GetRenderer()->GetUtils()->GetSphereGeom(gRenderSys->GetRenderer()));
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
		mesh->SetGeometry(gRenderSys->GetRenderer()->GetUtils()->GetSphereGeom(gRenderSys->GetRenderer()));
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
		mesh->SetGeometry(gRenderSys->GetRenderer()->GetUtils()->GetSphereGeom(gRenderSys->GetRenderer()));
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
		const Mesh::PTR& mesh = gRenderSys->GetRenderer()->GetMesh("assets/flop.fbx");
		MeshComponent* rootMesh = MeshComponent::Build(mesh, flopa);
		if (rootMesh) {
			Texture tex(0, L"assets/flopTex.png", gRenderSys->GetRenderer());
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
		const Mesh::PTR& mesh = gRenderSys->GetRenderer()->GetMesh("assets/cheese.fbx");
		MeshComponent* rootMesh = MeshComponent::Build(mesh, flopa);
		if (rootMesh) {
			Texture tex(0, L"assets/cheeseTex.jpg", gRenderSys->GetRenderer());
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
#endif
}

void Game::UnloadData()
{
	/*while (!components.empty())
	{
		delete components.back();
	}*/
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
		SetWindowText(wndGetHWND(gRenderSys->GetRenderer()->GetWindow()), text);
#endif

		frameNum = 0;
	}

#ifdef EDITOR
	if (!UIDebug::start_simulation)
	{
		gRenderSys->cameraManager.SetActiveCamera(editorCam->GetCameraHandle());
		if (!UIDebug::outliner.gizmo_focused)
		{
			editorCam->Update(deltaTime);
		}
		return;
	}
#endif // EDITOR

	if (!gScriptSys->Update(deltaTime)) {
        std::cout << "Failed to execute script update function." << std::endl;
   	}

	for (const auto& pScene : scenes) {
		ComponentStorage& storage = pScene->GetStorage();
		storage.BeginUpdate();
		for (Component* comp : storage.GetComponents()) {
			comp->Update(deltaTime);
		}
		storage.EndUpdate();
	}
}

void Game::GenerateOutput()
{
	// should be called from engine side
	// gRenderSys->GetRenderer()->Draw();
}

void Game::AddComponent(Component* comp)
{
	// TEMP: add component to the first loaded scene
	scenes[0]->GetStorage().AddComponent(comp);
}

void Game::RemoveComponent(Component* comp) 
{
	// TEMP: remove component from the first loaded scene
	scenes[0]->GetStorage().RemoveComponent(comp);
}
