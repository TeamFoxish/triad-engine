#pragma once

#include "game/Component.h"
#include "render/light/LightsStorage.h"

class PointLightComponent : public Component {
public:
	PointLightComponent(Game* game, Compositer* parent);
	~PointLightComponent();

	PointLight& GetLightSource() { return LightsStorage::Instance().pointLights.Get(handle); }

protected:
	LightsStorage::StorageImpl<PointLight>::Handle handle;
};
