#include "ThirdPersonCamera.h"

#include "CompositeComponent.h"
#include "game/Game.h"
#include "input/InputDevice.h"
#include <iostream>
#include <cmath>

#include "shared/SharedStorage.h"
#include "render/RenderSystem.h"
#include "logs/Logs.h"

ThirdPersonCamera::ThirdPersonCamera(Game* game, const Camera::Params& params, Compositer* target, Compositer* parent)
	: CameraComponent(game, params, parent)
	, mPitchSpeed(0.0f)
	, mMaxPitch(Math::Pi / 2.1f)
	, mPitch(0.0f)
	, targetC(target)
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

	gRenderSys->cameraManager.SetActiveCamera(handle);

	angleY += mPitchSpeed * deltaTime;
	angleY = Math::Clamp(angleY, -mMaxPitch, mMaxPitch);

	Math::Vector3 playerPos = targetC->GetPosition();
	Math::Vector3 camPos;
	camPos.x = playerPos.x + sin(angleX) * radius;
	camPos.y = playerPos.y + sin(angleY) * radius;
	camPos.z = playerPos.z + cos(angleX) * radius;
	
	Math::Transform& parentTrs = SharedStorage::Instance().transforms.AccessWrite(parentC->GetTransformHandle());
	parentTrs.SetPosition(camPos);
	Math::Vector3 viewDir = playerPos - camPos;
	viewDir.Normalize();
	Math::Vector3 left = viewDir;
	left.y = 0;
	left.Normalize();
	Math::Quaternion rot = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitY, Math::Pi / 2);
	left = Math::Vector3::Transform(left, rot);
	left.Normalize();
	Math::Vector3 up = viewDir.Cross(left);
	up.Normalize();

	const float dot = up.Dot(viewDir);
	
	//Math::Quaternion camRot = Math::Quaternion::LookRotation(viewDir, up);
	//parentTrs.SetRotation(camRot);

	//parentTrs.SetPosition(camPos);
	/*Math::Vector3 viewDir = playerPos - camPos;
	viewDir.Normalize();
	Math::Vector3 right = viewDir;
	right.y = 0.0f;
	right.Normalize();
	Math::Quaternion rot = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitY, Math::Pi / 2);
	right = -Math::Vector3::Transform(right, rot);
	Math::Vector3 up = right.Cross(viewDir);
	Math::Quaternion camRot = Math::Quaternion::LookRotation(viewDir, up);
	parentTrs.SetRotation(camRot);*/
	//Math::Quaternion q = Math::Quaternion::CreateFromAxisAngle(parent->GetRight(), angleY);

	//Math::Vector3 viewDir = playerPos - camPos;
	//viewDir.Normalize();
	//Math::Vector3 right = viewDir;
	//right.z = 0.0f;
	//right.Normalize();
	//Math::Quaternion rot = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitZ, Math::Pi / 2);
	//right = -Math::Vector3::Transform(right, rot);
	//Math::Vector3 viewForward = viewDir;
	//Math::Vector3 target = camPos + viewForward * 100.0f;
	////Math::Vector3 up = Math::Vector3::Transform(Math::Vector3::UnitZ, q);
	//Math::Vector3 up = right.Cross(viewDir);

	//// Create look at matrix, set as view
	const Math::Matrix viewMatr = Math::Matrix::CreateLookAt(camPos, camPos + viewDir * 100.0f, up);
	////Math::Transform& parentTrs = SharedStorage::Instance().transforms.AccessWrite(parentC->GetTransformHandle());
	parentTrs.SetMatrix(viewMatr.Invert());

	mPitchSpeed = 0.0f;
}

void ThirdPersonCamera::ProceedInput(InputDevice* inpDevice)
{
	Math::Vector3 forward = parentC->GetForward();
	forward.y = 0.0f;
	forward.Normalize();
	Math::Vector3 right = forward;
	Math::Quaternion rot = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitY, -Math::Pi / 2);
	right = Math::Vector3::Transform(right, rot);
	forward.Normalize();
	right.Normalize();
	if (inpDevice->IsKeyDown(Keys::D)) {
		Math::Vector3 pos = targetC->GetPosition() + right * 0.1f;
		targetC->SetPosition(pos);
	}
	if (inpDevice->IsKeyDown(Keys::A)) {
		Math::Vector3 pos = targetC->GetPosition() - right * 0.1f;
		targetC->SetPosition(pos);
	}
	if (inpDevice->IsKeyDown(Keys::W)) {
		Math::Vector3 pos = targetC->GetPosition() + forward * 0.1f;
		targetC->SetPosition(pos);
	}
	if (inpDevice->IsKeyDown(Keys::S)) {
		Math::Vector3 pos = targetC->GetPosition() - forward * 0.1f;
		targetC->SetPosition(pos);
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
		angleX -= angle;
		/*Math::Quaternion rot = parentC->GetRotation();
		Math::Quaternion inc = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitZ, angle);
		rot = Math::Quaternion::Concatenate(rot, inc);*/
		//parentC->SetRotation(rot);
	}
	if (abs(event.Offset.y) > 0.01f) {
		mPitchSpeed = (event.Offset.y / MAX_MOUSE_SPEED) * MAX_PITCH_SPEED;
	}
}
