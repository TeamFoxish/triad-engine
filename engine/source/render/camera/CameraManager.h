#pragma once

#include "CameraStorage.h"
#include "misc/Delegates.h"

struct RenderContext;

class CameraManager {
public:
    CameraManager() = default;
    CameraManager(const CameraManager&) = delete;
    CameraManager(CameraManager&&) = delete;
    ~CameraManager() = default;

    void Init();
    void Term();

    bool HasActiveCamera() const { return activeCamera.id_ >= 0; }
    CameraStorage::Handle GetActiveCameraHandle() const { return activeCamera; }
    CameraStorage::CameraEntry& GetActiveCamera() const;
    void SetActiveCamera(CameraStorage::Handle handle);

    const Math::Matrix& GetViewProjTransposed();

    void UpdateCameras();

private:
    Math::Matrix CalculateViewProjTransposed() const;

    void HandleViewportUpdate(int width, int height);

private:
    CameraStorage::Handle activeCamera;
    Math::Matrix viewProjTransposed;
    bool viewProjUpdate = false; // in case anyone asks for matrix before the UpdateCameras

    DelegateHandle viewportUpdateHandle;
};
