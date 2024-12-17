#pragma once

#include "components/CameraComponent.h"

class InputDevice;
class Game;

class EditorCamera {
public:
	EditorCamera(Game* game, const CameraParams& params);
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

	Math::Vector3 GetForwardVector() const;
	Math::Vector3 GetRight() const;

public:
	void SetProjectionMatrix(const Math::Matrix& matr) { projMatr = matr; }
	const Math::Matrix& GetProjectionMatrix() const { return projMatr; }
	const Math::Matrix& GetViewMatrix() const { return viewMatr; }

private:
	Game* _game;

	Math::Vector3 cameraPos;
	Math::Quaternion cameraRot;

private:
	// Rotation/sec speed of pitch
	float mPitchSpeed;
	// Maximum pitch deviation from forward
	float mMaxPitch;
	// Current pitch
	float mPitch;

private:
	void SetViewMatrix(const Math::Matrix& view);

	Math::Matrix projMatr;
	Math::Matrix viewMatr;

	std::unique_ptr<CameraParams> cachedParams; // TEMP

	DelegateHandle viewportUpdateHandle;
};
