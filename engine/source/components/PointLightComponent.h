#pragma once

#include "core/Component.h"
#include "render/Lights.h"

class PointLightComponent : public Component {
public:
	PointLightComponent(Game* game, Compositer* parent);

	PointLight& GetLightSource() { return pointLight; }

protected:
	PointLight pointLight;
};
