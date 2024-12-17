#pragma once

#include <vector>
#include <string>
#include <chrono>

#include "Component.h"

class Scene;
class CameraComponent;
class EditorCamera;

class Game {
	friend Component::Component(Game*, Compositer*);
	friend Component::~Component();
	friend class PlayerBall; // TODO: remove this !!!

public:
	Game();
	~Game();

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

#ifdef EDITOR
	EditorCamera* GetEditorCamera() const { return editorCam; }
#endif // EDITOR

private:
	void AddComponent(Component* comp);
	void RemoveComponent(Component* comp);

public:
	class PlayerBall* player = nullptr;

	// TODO: replace with unified scene root
	std::vector<std::unique_ptr<Scene>> scenes;

private:
	class CameraComponent* camera = nullptr;

#ifdef EDITOR
	class EditorCamera* editorCam = nullptr;
#endif // EDITOR

	std::chrono::time_point<std::chrono::steady_clock> prevTime;
	float totalTime = 0.0f;
	float deltaTime = 0.0f;
	int frameNum = 0;

public:
	bool isRunning = true;
};
