#include "CameraManager.h"

#include "render/RenderStorage.h"
#include "render/RenderContext.h"

#include "shared/SharedStorage.h"
#include "runtime/EngineRuntime.h"

#include "logs/Logs.h"

void CameraManager::Init()
{
    viewportUpdateHandle = gViewportResized.AddLambda([this](int width, int height) {
        HandleViewportUpdate(width, height);
	});
}

void CameraManager::Term()
{
    activeCamera = CameraStorage::Handle{};
    gViewportResized.Remove(viewportUpdateHandle);
}

CameraStorage::CameraEntry& CameraManager::GetActiveCamera() const
{
    assert(HasActiveCamera());
    return RenderStorage::Instance().cameras.Get(activeCamera);
}

void CameraManager::SetActiveCamera(CameraStorage::Handle handle)
{
    activeCamera = handle;
    viewProjUpdate = true;
}

const Math::Matrix& CameraManager::GetViewProjTransposed()
{
    if (viewProjUpdate) {
        viewProjUpdate = false;
        viewProjTransposed = CalculateViewProjTransposed();
    }
    return viewProjTransposed;
}

void CameraManager::UpdateCameras()
{
    if (HasActiveCamera()) {
        CameraStorage::CameraEntry& entry = GetActiveCamera();
        const Math::Transform& camTrs = SharedStorage::Instance().transforms.AccessRead(entry.transform);
        entry.camera.UpdateView(camTrs);
        if (entry.updateProjection) {
            entry.camera.UpdateProjection(entry.params);
            entry.updateProjection = false;
        }
    }

    // update view-projection matrix for renderer
    viewProjUpdate = false;
    viewProjTransposed = CalculateViewProjTransposed();
}

Math::Matrix CameraManager::CalculateViewProjTransposed() const
{
    if (!HasActiveCamera()) {
        LOG_ERROR("failed to calculate view-projection matrix. no active camera was set");
        return Math::Matrix{};
    }
    const Camera& cam = GetActiveCamera().camera;
    return (cam.GetViewMatrix() * cam.GetProjectionMatrix()).Transpose();
}

void CameraManager::HandleViewportUpdate(int width, int height)
{
    for (CameraStorage::CameraEntry& entry : RenderStorage::Instance().cameras.storage) {
        entry.params.width = (float)width;
        entry.params.height = (float)height;
        entry.camera.UpdateProjection(entry.params);
    }
}
