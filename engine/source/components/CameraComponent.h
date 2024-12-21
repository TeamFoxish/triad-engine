#pragma once

#include "game/Component.h"
#include "math/Math.h"

#include <memory>

#include "render/camera/CameraStorage.h"

class CameraComponent : public Component {
public:
	CameraComponent(Game* game, const Camera::Params& params, Compositer* parent = nullptr);
	~CameraComponent();

protected:
	CameraStorage::Handle handle;
};
