#pragma once

#include <memory>
#include <vector>
#include "math/Math.h"

struct GeometryData;

class Mesh {
	friend class MeshLoader;

public:
	using PTR = std::shared_ptr<Mesh>;

	struct MeshNode {
		std::vector<std::shared_ptr<GeometryData>> geoms;
		Math::Matrix localMatr;
	};

	~Mesh();

	const std::vector<MeshNode>& GetNodes() const { return nodes; }

	static Mesh::PTR CreateFromGeom(const std::shared_ptr<GeometryData>& geom);

protected:
	std::vector<MeshNode> nodes;
};
