#pragma once

#include "render/camera/CameraStorage.h"

class InputDevice;
class Game;

class EditorCamera {
public:
	EditorCamera(Game* game, const Camera::Params& params);
	~EditorCamera();

	void Initialize();
	void Update(float deltaTime);
	void ProceedInput(InputDevice* inpDevice);
	void ProceedMouseInput(const struct MouseMoveEventArgs& event);

	float GetPitch() const { return mPitch; }
	float GetPitchSpeed() const { return mPitchSpeed; }
	float GetMaxPitch() const { return mMaxPitch; }

	void SetPitchSpeed(float speed) { mPitchSpeed = speed; }
	void SetMaxPitch(float pitch) { mMaxPitch = pitch; }

	const Math::Matrix& GetViewMatrix() const;
	const Math::Matrix& GetProjectionMatrix() const;

	CameraStorage::Handle GetCameraHandle() const { return handle; }

private:
	Game* _game;

private:
	// Rotation/sec speed of pitch
	float mPitchSpeed;
	// Maximum pitch deviation from forward
	float mMaxPitch;
	// Current pitch
	float mPitch;

private:
	TransformStorage::Handle transform;
	CameraStorage::Handle handle;
};
