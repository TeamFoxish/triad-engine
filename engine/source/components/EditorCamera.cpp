#include "EditorCamera.h"

#include "game/Game.h"
#include "input/InputDevice.h"
#include "runtime/EngineRuntime.h"
#include "render/Renderer.h"
#include "render/RenderSystem.h"
#include "render/RenderStorage.h"
#include "shared/SharedStorage.h"

EditorCamera::EditorCamera(Game* game, const Camera::Params& params)
    : _game(game)
	, mPitchSpeed(0.0f)
    , mMaxPitch(Math::Pi / 2.1f)
    , mPitch(0.0f)
	, transform(SharedStorage::Instance().transforms.Add())
	, handle(RenderStorage::Instance().cameras.Add(params, transform))
{
}

EditorCamera::~EditorCamera()
{
	RenderStorage::Instance().cameras.Remove(handle);
	SharedStorage::Instance().transforms.Remove(transform);
}

void EditorCamera::Initialize()
{
    auto dh = globalInputDevice->MouseMove.AddRaw(this, &EditorCamera::ProceedMouseInput);
}

void EditorCamera::Update(float deltaTime)
{
}

void EditorCamera::ProceedInput(InputDevice* inpDevice)
{
	if (!globalInputDevice->IsKeyHold(Keys::RightButton)) {
		return;
	}

	Math::Transform& camTrs = SharedStorage::Instance().transforms.AccessWrite(transform);
	Math::Vector3 camPos = camTrs.GetPosition();
	const Math::Vector3 forward = Math::Vector3::Transform(Math::Vector3::Forward, camTrs.GetRotation());
	const Math::Vector3 right = Math::Vector3::Transform(Math::Vector3::Right, camTrs.GetRotation());
	if (inpDevice->IsKeyHold(Keys::D)) {
		camPos = camPos + right * 0.1f;
	}
	if (inpDevice->IsKeyHold(Keys::A)) {
		camPos = camPos - right * 0.1f;
	}
	if (inpDevice->IsKeyHold(Keys::W)) {
		camPos = camPos + forward * 0.1f;
	}
	if (inpDevice->IsKeyHold(Keys::S)) {
		camPos = camPos - forward * 0.1f;
	}
	camTrs.SetPosition(camPos);
}

void EditorCamera::ProceedMouseInput(const MouseMoveEventArgs& event)
{
	if (!globalInputDevice->IsKeyHold(Keys::RightButton)) {
		return;
	}

	constexpr int MAX_MOUSE_SPEED = 25;
	const float MAX_ANGULAR_SPEED = Math::Pi * 8;
	const float MAX_PITCH_SPEED = Math::Pi * 8;
	const float dt = _game->GetDeltaTime();
	{
		Math::Transform& camTrs = SharedStorage::Instance().transforms.AccessWrite(transform);
		Math::Quaternion camRot = camTrs.GetRotation();
		Math::Vector3 euler = camRot.ToEuler();
		Math::Vector2 angularSpeed{event.Offset.x, event.Offset.y};
		Math::Vector2 angle = -angularSpeed * dt;
		const float sensitivity = 1.0f;
		static float rotationX = 0.0f;
		rotationX = rotationX + angle.x * sensitivity;

		static float rotationY = 0.0f;
		rotationY += angle.y * sensitivity;
		rotationY = Math::Clamp(rotationY, -mMaxPitch, mMaxPitch);

		camRot = Math::Quaternion::CreateFromYawPitchRoll(rotationX, rotationY, 0.0f);
		camTrs.SetRotation(camRot);
	}
}

const Math::Matrix& EditorCamera::GetViewMatrix() const
{
	return RenderStorage::Instance().cameras.Get(handle).camera.GetViewMatrix();
}

const Math::Matrix& EditorCamera::GetProjectionMatrix() const
{
	return RenderStorage::Instance().cameras.Get(handle).camera.GetProjectionMatrix();
}
