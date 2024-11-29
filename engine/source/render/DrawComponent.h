#pragma once

#include "game/Component.h"
#include <cstdint>
#include <memory>

class Renderer;

class Material;
struct GeometryData;

class DrawComponent : public Component
{
public:
	DrawComponent(Game* game, Compositer* compositer = nullptr);
	~DrawComponent();

	virtual void Draw(Renderer* renderer) = 0;
};
