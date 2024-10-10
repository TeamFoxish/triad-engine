#pragma once

#include "core/Component.h"
#include "render/Lights.h"

class DirectionalLightComponent : public Component {
public:
	DirectionalLightComponent(Game* game, Compositer* parent);

	DirectionalLight& GetLightSource() { return dirLight; }

protected:
	DirectionalLight dirLight;
};
