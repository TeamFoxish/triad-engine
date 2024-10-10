#pragma once

#include "render/DrawComponent.h"
#include "core/Math.h"


class QuadComponent : public DrawComponent {
public:
	QuadComponent(Game* game, Compositer* compositer = nullptr);

	void Initialize(Compositer* parent = nullptr) override { parentRef = parent; }
	void Draw(Renderer* renderer) override;

	void SetColor(const Math::Color& _color) { color = _color; }
	void SetSize(Math::Vector2 _size) { size = _size; }

private:
	Compositer* parentRef = nullptr;

	Math::Color color{1.0f, 1.0f, 1.0f, 1.0f};
	Math::Vector2 size{100.0f, 100.0f};
};
