#include "EditorCamera.h"

#include "game/Game.h"
#include "input/InputDevice.h"
#include "runtime/EngineRuntime.h"
#include "render/Renderer.h"
#include "render/RenderSystem.h"
#include "render/RenderStorage.h"
#include "shared/SharedStorage.h"

#include "logs/Logs.h"

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
	Math::Transform& camTrs = SharedStorage::Instance().transforms.AccessWrite(transform);
	Math::Quaternion camRot = camTrs.GetRotation();
	const Math::Vector3 euler = camRot.ToEuler();
	//mPitch = euler.y;
	LOG_DEBUG("pitch = {}", mPitch);
	mPitch += mPitchSpeed * deltaTime;
	LOG_DEBUG("pitch222 = {}", mPitch);
	mPitch = Math::Clamp(mPitch, -mMaxPitch, mMaxPitch);
	//const float diff = clamped - euler.y;
	//LOG_DEBUG("pitch_clamped = {}", diff);break();
	//camRot = Math::Quaternion::Concatenate(camRot, Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitX, diff));
	camRot = Math::Quaternion::CreateFromYawPitchRoll(euler.x, mPitch, euler.z); // roll, pitch, yaw
	//camRot = Math::Quaternion::LookRotation(Math::Vector3::Transform(Math::Vector3::Forward, Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitX, mPitch)), );
	//camTrs.SetRotation(camRot);
	//camRot = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitY, -mPitch);

	const Math::Vector3 camPos = camTrs.GetPosition();
	const Math::Vector3 left = Math::Vector3::Transform(Math::Vector3::UnitY, camRot); // left
	//const Math::Quaternion q = Math::Quaternion::CreateFromAxisAngle(left, -mPitch);
	Math::Vector3 forward = Math::Vector3::Transform(Math::Vector3::UnitX, camRot);
	//forward = Math::Vector3::Transform(forward, q);
	
	Math::Vector3 target = camPos + forward * 100.0f;
	Math::Vector3 up = Math::Vector3::Transform(Math::Vector3::UnitZ, camRot);

	// Create look at matrix, set as view
	//const Math::Matrix viewMatr = Math::Matrix::CreateLookAt(camPos, target, up);
	//camTrs.SetMatrix(Math::Matrix::CreateLookAt(camPos, target, up));
	//Math::Matrix view2;
	//view2._11 = 1.0f; // L
	//view2._21 = 0.0f;
	//view2._31 = 0.0f;

	//view2._12 = 0.0f; // U
	//view2._22 = 1.0f;
	//view2._32 = 0.0f;

	//view2._13 = 0.0f; // F
	//view2._23 = 0.0f;
	//view2._33 = 1.0f;

	//view2._41 = camPos.x; // T
	//view2._42 = camPos.y;
	//view2._43 = camPos.z;

	//view2._14 = 0.0f;
	//view2._24 = 0.0f;
	//view2._34 = 0.0f;
	//view2._44 = 1.0f;

	Math::Matrix view2;
	view2._11 = forward.x; // L
	view2._21 = forward.y;
	view2._31 = forward.z;

	view2._12 = left.x; // U
	view2._22 = left.y;
	view2._32 = left.z;

	view2._13 = up.x; // F
	view2._23 = up.y;
	view2._33 = up.z;

	view2._41 = camPos.x; // T
	view2._42 = camPos.y;
	view2._43 = camPos.z;

	view2._14 = 0.0f;
	view2._24 = 0.0f;
	view2._34 = 0.0f;
	view2._44 = 1.0f;

	//camTrs.SetMatrix(view2);
	mPitchSpeed = 0.0f;
}

void EditorCamera::ProceedInput(InputDevice* inpDevice)
{
	Math::Transform& camTrs = SharedStorage::Instance().transforms.AccessWrite(transform);
	Math::Vector3 camPos = camTrs.GetPosition();
	const Math::Vector3 forward = Math::Vector3::Transform(Math::Vector3::Forward, camTrs.GetRotation());
	const Math::Vector3 right = Math::Vector3::Transform(Math::Vector3::Right, camTrs.GetRotation());
	if (inpDevice->IsKeyDown(Keys::D)) {
		camPos = camPos + right * 0.1f;
	}
	if (inpDevice->IsKeyDown(Keys::A)) {
		camPos = camPos - right * 0.1f;
	}
	if (inpDevice->IsKeyDown(Keys::W)) {
		camPos = camPos + forward * 0.1f;
	}
	if (inpDevice->IsKeyDown(Keys::S)) {
		camPos = camPos - forward * 0.1f;
	}
	camTrs.SetPosition(camPos);
}

void EditorCamera::ProceedMouseInput(const MouseMoveEventArgs& event)
{
	constexpr int MAX_MOUSE_SPEED = 25;
	const float MAX_ANGULAR_SPEED = Math::Pi * 8;
	const float MAX_PITCH_SPEED = Math::Pi * 8;
	const float dt = _game->GetDeltaTime();
	//if (abs(event.Offset.x) > 0.01f) {
	//	Math::Transform& camTrs = SharedStorage::Instance().transforms.AccessWrite(transform);
	//	Math::Quaternion camRot = camTrs.GetRotation();
	//	Math::Vector3 euler = camRot.ToEuler();
	//	//const float angularSpeed = (event.Offset.x / MAX_MOUSE_SPEED) * MAX_ANGULAR_SPEED;
	//	const float angularSpeed = event.Offset.x;
	//	float angle = angularSpeed * dt;
	//	euler.z += angle;
	//	//LOG_DEBUG("angle = {}", angle);
	//	Math::Quaternion inc = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitZ, angle);
	//	//camRot = Math::Quaternion::Concatenate(camRot, inc);
	//	camRot = Math::Quaternion::CreateFromYawPitchRoll(euler);
	//	camTrs.SetRotation(camRot);
	//}
	//if (abs(event.Offset.y) > 0.01f) {
	//	mPitchSpeed = (event.Offset.y / MAX_MOUSE_SPEED) * MAX_PITCH_SPEED;
	//}
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
		//Math::Matrix rotMatr = Math::Matrix::CreateRotationX(rotationY) * Math::Matrix::CreateRotationZ(rotationX);
		//camRot = Math::Quaternion::CreateFromRotationMatrix(rotMatr);
		//camRot = Math::Quaternion::CreateFromYawPitchRoll(Math::Vector3{rotationY, 0.0f, rotationX});
		//transform.localEulerAngles = new Vector3(-rotationY, rotationX, 0);
		camTrs.SetRotation(camRot);
	}
	/*else if (axes == RotationAxes.MouseX)
	{
		transform.Rotate(0, Input.GetAxis("Mouse X") * sensitivityX, 0);
	}
	else
	{
		rotationY += Input.GetAxis("Mouse Y") * sensitivityY;
		rotationY = Mathf.Clamp(rotationY, minimumY, maximumY);

		transform.localEulerAngles = new Vector3(-rotationY, transform.localEulerAngles.y, 0);
	}*/
}

const Math::Matrix& EditorCamera::GetViewMatrix() const
{
	return RenderStorage::Instance().cameras.Get(handle).camera.GetViewMatrix();
}

const Math::Matrix& EditorCamera::GetProjectionMatrix() const
{
	return RenderStorage::Instance().cameras.Get(handle).camera.GetProjectionMatrix();
}
