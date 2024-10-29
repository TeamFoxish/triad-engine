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

	void Initialize(Compositer* parent = nullptr) override;
	virtual void Draw(Renderer* renderer);

	std::weak_ptr<Material> GetMaterial() const { return material; }
	void SetMaterial(const std::shared_ptr<Material> _material) { material = _material; }
	void SetGeometry(const std::shared_ptr<GeometryData> _geom) { geom = _geom; }

private:
	std::shared_ptr<Material> material;
	std::shared_ptr<GeometryData> geom;  // TODO: remove geom?
};
