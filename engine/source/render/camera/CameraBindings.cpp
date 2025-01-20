#include "CameraBindings.h"

#include "scripts/ScriptSystem.h"

#include "render/RenderStorage.h"
#include "render/RenderSystem.h"
#include "render/RenderContext.h"
#include "shared/MathScriptBindings.h"
#include "shared/SharedStorage.h"

class CCamera : public CNativeObject {
	friend void RegisterCameraBindings();

public:
	CCamera() = default;

	CCamera(const TransformStorage::Handle transform) 
	{
		Camera::Params params;
		RenderContext& ctx = gRenderSys->GetContext();
		params.width = gRenderSys->GetContext().viewport.width;
		params.height = gRenderSys->GetContext().viewport.height;
		handle = RenderStorage::Instance().cameras.Add(params, transform);
	}

	CCamera(const CCamera& other)
	{
		*this = other;
	}

	~CCamera() 
	{
		if (gRenderSys->cameraManager.HasActiveCamera() && gRenderSys->cameraManager.GetActiveCameraHandle() == handle) {
			gRenderSys->cameraManager.SetActiveCamera(CameraStorage::Handle{});
		}
		RenderStorage::Instance().cameras.Remove(handle);
	}

	CCamera& operator=(const CCamera& other) 
	{
		// clone camera entry
		const CameraStorage::CameraEntry& entry = other.GetCamera();
		handle = RenderStorage::Instance().cameras.Add(entry.params, entry.transform);
		return *this;
	}

	CameraStorage::CameraEntry& GetCamera() const { return RenderStorage::Instance().cameras.Get(handle); }

	void MakePerspective() 
	{
		CameraStorage::CameraEntry& entry = GetCamera();
		entry.params.type = Camera::Params::Perspective;
		entry.updateProjection = true;
	}

	void MakeOrthographic()
	{
		CameraStorage::CameraEntry& entry = GetCamera();
		entry.params.type = Camera::Params::Orthographic;
		entry.updateProjection = true;
	}

	void SetFov(float fovDeg) 
	{
		CameraStorage::CameraEntry& entry = GetCamera();
		entry.params.fov = fovDeg;
		entry.updateProjection = true;
	}

	void SetLookTransform(const Math::Vector3& camPos, const Math::Vector3& target, const Math::Vector3& up) 
	{
		const CameraStorage::CameraEntry& entry = GetCamera();
		Math::Transform& camTrs = SharedStorage::Instance().transforms.AccessWrite(entry.transform);
		camTrs.SetMatrix(Math::Matrix::CreateLookAt(camPos, target, up).Invert());
	}

	void SetActive() 
	{
		gRenderSys->cameraManager.SetActiveCamera(handle);
	}

	static void CreateDefault(CCamera* self) { new(self) CCamera(); }
	static void CreateFromTransform(CCamera* self, const CTransformHandle& transform) { new(self) CCamera(transform.GetHandle()); }
	static void CreateCopy(CCamera* self, const CCamera& other) { new(self) CCamera(other); }
	static void Destroy(CCamera* self) { self->~CCamera(); }

private:
	CameraStorage::Handle handle;
	bool activateAtInit = false;
};

void RegisterCameraBindings()
{
	auto engine = gScriptSys->GetRawEngine();
	int r;

	r = engine->RegisterObjectType("Camera", sizeof(CCamera), asOBJ_VALUE | asGetTypeTraits<CCamera>() | asOBJ_APP_CLASS_MORE_CONSTRUCTORS); assert(r >= 0);
	CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
	r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CCamera::CreateDefault), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_CONSTRUCT, "void f(const Math::Transform@+)", asFUNCTION(CCamera::CreateFromTransform), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_CONSTRUCT, "void f(Camera &in)", asFUNCTION(CCamera::CreateCopy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Camera", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CCamera::Destroy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Camera", "Camera &opAssign(Camera &in)", asMETHODPR(CCamera, operator=, (const CCamera&), CCamera&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Camera", "void MakePerspective()", asMETHOD(CCamera, MakePerspective), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Camera", "void MakeOrthographic()", asMETHOD(CCamera, MakeOrthographic), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Camera", "void SetFov(float fovDeg)", asMETHOD(CCamera, SetFov), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Camera", "void SetLookTrasnform(const Math::Vector3 &in camPos, const Math::Vector3 &in target, const Math::Vector3 &in up)", asMETHOD(CCamera, SetLookTransform), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Camera", "void SetActive()", asMETHOD(CCamera, SetActive), asCALL_THISCALL); assert(r >= 0);
}
