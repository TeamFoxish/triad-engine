#include "FPSCamera.h"

#include "CompositeComponent.h"
#include "game/Game.h"
#include "input/InputDevice.h"
#include <iostream>


FPSCamera::FPSCamera(Game* game, const CameraParams& params, Compositer* parent)
	: CameraComponent(game, params, parent)
	, mPitchSpeed(0.0f)
	, mMaxPitch(Math::Pi / 2.1f)
	, mPitch(0.0f)
	, parentC(parent)
{
}

void FPSCamera::Initialize(Compositer* parent)
{
	globalInputDevice->MouseMove.AddRaw(this, &FPSCamera::ProceedMouseInput);
}

void FPSCamera::Update(float deltaTime, Compositer* parent)
{
	// Call parent update (doesn't do anything right now)
	CameraComponent::Update(deltaTime);
	if (!parent) {
		return;
	}

	Math::Vector3 cameraPos = parent->GetPosition();

	mPitch += mPitchSpeed * deltaTime;
	mPitch = Math::Clamp(mPitch, -mMaxPitch, mMaxPitch);
	Math::Quaternion q = Math::Quaternion::CreateFromAxisAngle(parent->GetRight(), mPitch);

	Math::Vector3 viewForward = GetForwardVector(parent);
	Math::Vector3 target = cameraPos + viewForward * 100.0f;
	Math::Vector3 up = Math::Vector3::Transform(Math::Vector3::UnitZ, q);

	// Create look at matrix, set as view
	Math::Matrix view = Math::Matrix::CreateLookAt(cameraPos, target, up);
	SetViewMatrix((view * projMatr).Transpose());
	mPitchSpeed = 0.0f;
}

void FPSCamera::ProceedInput(InputDevice* inpDevice)
{
	Math::Vector3 forward = GetForwardVector(parentC);
	Math::Vector3 right = parentC->GetRight();
	forward.Normalize();
	right.Normalize();
	if (inpDevice->IsKeyDown(Keys::D)) {
		Math::Vector3 pos = parentC->GetPosition() - right * 0.1f;
		parentC->SetPosition(pos);
	}
	if (inpDevice->IsKeyDown(Keys::A)) {
		Math::Vector3 pos = parentC->GetPosition() + right * 0.1f;
		parentC->SetPosition(pos);
	}
	if (inpDevice->IsKeyDown(Keys::W)) {
		Math::Vector3 pos = parentC->GetPosition() + forward * 0.1f;
		parentC->SetPosition(pos);
	}
	if (inpDevice->IsKeyDown(Keys::S)) {
		Math::Vector3 pos = parentC->GetPosition() - forward * 0.1f;
		parentC->SetPosition(pos);
	}
}

void FPSCamera::ProceedMouseInput(const MouseMoveEventArgs& event)
{
	constexpr int MAX_MOUSE_SPEED = 25;
	const float MAX_ANGULAR_SPEED = Math::Pi * 8;
	const float MAX_PITCH_SPEED = Math::Pi * 8;
	const float dt = GetGame()->GetDeltaTime();
	if (abs(event.Offset.x) > 0.01f) {
		const float angularSpeed = (event.Offset.x / MAX_MOUSE_SPEED) * MAX_ANGULAR_SPEED;
		float angle = -angularSpeed * dt;
		Math::Quaternion rot = parentC->GetRotation();
		Math::Quaternion inc = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitZ, angle);
		rot = Math::Quaternion::Concatenate(rot, inc);
		parentC->SetRotation(rot);
	}
	if (abs(event.Offset.y) > 0.01f) {
		mPitchSpeed = (event.Offset.y / MAX_MOUSE_SPEED) * MAX_PITCH_SPEED;
	}
}

Math::Vector3 FPSCamera::GetForwardVector(Compositer* parent) const
{
	const Math::Quaternion q = Math::Quaternion::CreateFromAxisAngle(parent->GetRight(), mPitch);
	return Math::Vector3::Transform(parent->GetForward(), q);
}
