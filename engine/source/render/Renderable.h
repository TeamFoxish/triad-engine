#pragma once

#include "mesh/Mesh.h"
#include "material/Material.h"
#include "shared/TransformStorage.h"

struct Renderable {
	struct DrawParams {
		bool isVisible = true;
		bool castShadows = true;
	};

	Renderable(uint32_t entityId, TransformStorage::Handle transform);
	~Renderable();

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
	const TransformStorage::Handle transform;
	DrawParams params;
	uint32_t entityId;
};
