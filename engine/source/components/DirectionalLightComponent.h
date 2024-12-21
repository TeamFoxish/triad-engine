#pragma once

#include "game/Component.h"
#include "render/light/LightsStorage.h"

class DirectionalLightComponent : public Component {
public:
	DirectionalLightComponent(Game* game, Compositer* parent);
	~DirectionalLightComponent();

	DirectionalLight& GetLightSource() { return LightsStorage::Instance().dirLights.Get(handle); }

protected:
	LightsStorage::StorageImpl<DirectionalLight>::Handle handle;
};
