#include "EditorCamera.h"

#include "game/Game.h"
#include "input/InputDevice.h"
#include "runtime/EngineRuntime.h"
#include "render/Renderer.h"
#include "render/RenderSystem.h"


EditorCamera::EditorCamera(Game* game, const CameraParams& params)
    : _game(game)
	, mPitchSpeed(0.0f)
    , mMaxPitch(Math::Pi / 2.1f)
    , mPitch(0.0f)
    , projMatr(params.MakeProjectionMatrix())
    , cachedParams(params.Clone())
    , cameraPos()
	, cameraRot()
{
    gViewportResized.AddLambda([this](int width, int height) {
        cachedParams->UpdateViewport(width, height);
        projMatr = cachedParams->MakeProjectionMatrix();
        });
}

EditorCamera::~EditorCamera()
{
    gViewportResized.Remove(viewportUpdateHandle);
}

void EditorCamera::Initialize()
{
    auto dh = globalInputDevice->MouseMove.AddRaw(this, &EditorCamera::ProceedMouseInput);
}

void EditorCamera::Update(float deltaTime)
{
	mPitch += mPitchSpeed * deltaTime;
	mPitch = Math::Clamp(mPitch, -mMaxPitch, mMaxPitch);
	Math::Quaternion q = Math::Quaternion::CreateFromAxisAngle(GetRight(), mPitch);

	Math::Vector3 viewForward = GetForwardVector();
	Math::Vector3 target = cameraPos + viewForward * 100.0f;
	Math::Vector3 up = Math::Vector3::Transform(Math::Vector3::UnitZ, q);

	// Create look at matrix, set as view
	viewMatr = Math::Matrix::CreateLookAt(cameraPos, target, up);
	SetViewMatrix((viewMatr * projMatr).Transpose());
	mPitchSpeed = 0.0f;
}

void EditorCamera::ProceedInput(InputDevice* inpDevice)
{
	Math::Vector3 forward = GetForwardVector();
	Math::Vector3 right = GetRight();
	forward.Normalize();
	right.Normalize();
	if (inpDevice->IsKeyDown(Keys::D)) {
		cameraPos = cameraPos - right * 0.1f;
	}
	if (inpDevice->IsKeyDown(Keys::A)) {
		cameraPos = cameraPos + right * 0.1f;
	}
	if (inpDevice->IsKeyDown(Keys::W)) {
		cameraPos = cameraPos + forward * 0.1f;
	}
	if (inpDevice->IsKeyDown(Keys::S)) {
		cameraPos = cameraPos - forward * 0.1f;
	}
}

void EditorCamera::ProceedMouseInput(const MouseMoveEventArgs& event)
{
	constexpr int MAX_MOUSE_SPEED = 25;
	const float MAX_ANGULAR_SPEED = Math::Pi * 8;
	const float MAX_PITCH_SPEED = Math::Pi * 8;
	const float dt = _game->GetDeltaTime();
	if (abs(event.Offset.x) > 0.01f) {
		const float angularSpeed = (event.Offset.x / MAX_MOUSE_SPEED) * MAX_ANGULAR_SPEED;
		float angle = -angularSpeed * dt;
		//Math::Quaternion rot = parentC->GetRotation();
		Math::Quaternion inc = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitZ, angle);
		cameraRot = Math::Quaternion::Concatenate(cameraRot, inc);
		//parentC->SetRotation(rot);
	}
	if (abs(event.Offset.y) > 0.01f) {
		mPitchSpeed = (event.Offset.y / MAX_MOUSE_SPEED) * MAX_PITCH_SPEED;
	}
}

Math::Vector3 EditorCamera::GetForwardVector() const
{
	const Math::Quaternion q = Math::Quaternion::CreateFromAxisAngle(GetRight(), mPitch);
	return Math::Vector3::Transform(Math::Vector3::Transform(Math::Vector3::UnitX, cameraRot), q);
}

Math::Vector3 EditorCamera::GetRight() const
{
	return Math::Vector3::Transform(Math::Vector3::UnitY, cameraRot);
}

void EditorCamera::SetViewMatrix(const Math::Matrix& view)
{
	gRenderSys->GetRenderer()->SetViewMatrix(view);
}
