#pragma once

#include <vector>
#include <string>
#include <chrono>

#include "Component.h"
#include "render/Renderer.h"

class Window;
class Renderer;
class PaddleComponent;
class CameraComponent;

class Game {
	friend Component::Component(Game*, Compositer*);
	friend Component::~Component();
	friend class PlayerBall; // TODO: remove this !!!

public:
	bool Initialize();
	void ProcessInput();
	void Shutdown();
	void UpdateFrame();

	void Restart();
	void LoadData();
	void UnloadData();
	void UpdateGame();
	void GenerateOutput();

	float GetDeltaTime() const { return deltaTime; }

	class CompositeComponent* GetCameraHolder();

	CameraComponent* GetActiveCamera() const { return camera; }
	void SetActiveCamera(CameraComponent* cam) { camera = cam; }

private:
	void AddComponent(Component* comp);
	void RemoveComponent(Component* comp);

private:
	std::vector<Component*> components;
	std::vector<Component*> pendingComponents;

	class PlayerBall* player = nullptr;
	class CameraComponent* camera = nullptr;

	std::chrono::time_point<std::chrono::steady_clock> prevTime;
	float totalTime = 0.0f;
	float deltaTime = 0.0f;
	int frameNum = 0;

	bool isUpdatingComponents = false;

public:
	bool isRunning = true;
};
