#include "CameraStorage.h"

#include "shared/SharedStorage.h"

auto CameraStorage::Add(const Camera::Params& params, TransformStorage::Handle transform) -> Handle
{
    const Math::Transform& cameraTrs = SharedStorage::Instance().transforms.AccessRead(transform);
    return storage.Add(CameraEntry{
        .camera = Camera(params, cameraTrs), 
        .params = params, 
        .transform = transform
    });
}
