#pragma once

#include "components/CameraComponent.h"


class FPSCamera : public CameraComponent {
public:
	FPSCamera(Game* game, const CameraParams& params, Compositer* parent = nullptr);

	void Initialize(Compositer* parent = nullptr) override;
	void Update(float deltaTime, Compositer* parent = nullptr) override;
	void ProceedInput(InputDevice* inpDevice) override;
	void ProceedMouseInput(const struct MouseMoveEventArgs& event);

	float GetPitch() const { return mPitch; }
	float GetPitchSpeed() const { return mPitchSpeed; }
	float GetMaxPitch() const { return mMaxPitch; }

	void SetPitchSpeed(float speed) { mPitchSpeed = speed; }
	void SetMaxPitch(float pitch) { mMaxPitch = pitch; }

	Math::Vector3 GetForwardVector(Compositer* parent) const override;

private:
	// Rotation/sec speed of pitch
	float mPitchSpeed;
	// Maximum pitch deviation from forward
	float mMaxPitch;
	// Current pitch
	float mPitch;

	Compositer* parentC;
};
