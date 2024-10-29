#include "ThirdPersonCamera.h"

#include "CompositeComponent.h"
#include "game/Game.h"
#include "input/InputDevice.h"
#include <iostream>
#include <cmath>


ThirdPersonCamera::ThirdPersonCamera(Game* game, const CameraParams& params, Compositer* parent)
	: CameraComponent(game, params, parent)
	, mPitchSpeed(0.0f)
	, mMaxPitch(Math::Pi / 2.1f)
	, mPitch(0.0f)
	, parentC(parent)
{
}

void ThirdPersonCamera::Initialize(Compositer* parent)
{
	globalInputDevice->MouseMove.AddRaw(this, &ThirdPersonCamera::ProceedMouseInput);
}

void ThirdPersonCamera::Update(float deltaTime, Compositer* parent)
{
	// Call parent update (doesn't do anything right now)
	CameraComponent::Update(deltaTime);
	if (!parent) {
		return;
	}

	angleY += mPitchSpeed * deltaTime;
	angleY = Math::Clamp(angleY, -mMaxPitch, mMaxPitch);

	Math::Vector3 playerPos = parent->GetPosition();
	Math::Vector3 camPos;
	camPos.x = playerPos.x - cos(angleX) * radius;
	camPos.y = playerPos.y + sin(angleX) * radius;
	camPos.z = playerPos.z + sin(angleY) * radius;
	
	//Math::Quaternion q = Math::Quaternion::CreateFromAxisAngle(parent->GetRight(), angleY);

	Math::Vector3 viewDir = playerPos - camPos;
	viewDir.Normalize();
	Math::Vector3 right = viewDir;
	right.z = 0.0f;
	right.Normalize();
	Math::Quaternion rot = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitZ, Math::Pi / 2);
	right = -Math::Vector3::Transform(right, rot);
	Math::Vector3 viewForward = viewDir;
	Math::Vector3 target = camPos + viewForward * 100.0f;
	//Math::Vector3 up = Math::Vector3::Transform(Math::Vector3::UnitZ, q);
	Math::Vector3 up = right.Cross(viewDir);

	// Create look at matrix, set as view
	Math::Matrix view = Math::Matrix::CreateLookAt(camPos, target, up);
	SetViewMatrix((view * projMatr).Transpose());
	mPitchSpeed = 0.0f;
}

void ThirdPersonCamera::ProceedInput(InputDevice* inpDevice)
{
	Math::Vector3 forward = GetForwardVector(parentC);
	forward.z = 0.0f;
	forward.Normalize();
	Math::Vector3 right = forward;
	Math::Quaternion rot = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitZ, -Math::Pi / 2);
	right = Math::Vector3::Transform(right, rot);
	forward.Normalize();
	right.Normalize();
	if (inpDevice->IsKeyDown(Keys::D)) {
		Math::Vector3 pos = parentC->GetPosition() + right * 0.1f;
		parentC->SetPosition(pos);
	}
	if (inpDevice->IsKeyDown(Keys::A)) {
		Math::Vector3 pos = parentC->GetPosition() - right * 0.1f;
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

void ThirdPersonCamera::ProceedMouseInput(const MouseMoveEventArgs& event)
{
	constexpr int MAX_MOUSE_SPEED = 25;
	const float MAX_ANGULAR_SPEED = Math::Pi * 8;
	const float MAX_PITCH_SPEED = Math::Pi * 8;
	const float dt = GetGame()->GetDeltaTime();
	if (abs(event.Offset.x) > 0.01f) {
		const float angularSpeed = (event.Offset.x / MAX_MOUSE_SPEED) * MAX_ANGULAR_SPEED;
		float angle = angularSpeed * dt;
		angleX += angle;
		/*Math::Quaternion rot = parentC->GetRotation();
		Math::Quaternion inc = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitZ, angle);
		rot = Math::Quaternion::Concatenate(rot, inc);*/
		//parentC->SetRotation(rot);
	}
	if (abs(event.Offset.y) > 0.01f) {
		mPitchSpeed = (event.Offset.y / MAX_MOUSE_SPEED) * MAX_PITCH_SPEED;
	}
}

Math::Vector3 ThirdPersonCamera::GetForwardVector(Compositer* parent) const
{
	const Math::Vector3 playerPos = parentC->GetPosition();
	const Math::Vector3 camPos = GetCameraPos();
	Math::Vector3 viewPos = playerPos - camPos;
	viewPos.Normalize();
	return viewPos;
}

Math::Vector3 ThirdPersonCamera::GetCameraPos() const
{
	Math::Vector3 playerPos = parentC->GetPosition();
	Math::Vector3 camPos;
	camPos.x = playerPos.x - cos(angleX) * radius;
	camPos.y = playerPos.y + sin(angleX) * radius;
	camPos.z = playerPos.z + sin(angleY) * radius;
	return camPos;
}
