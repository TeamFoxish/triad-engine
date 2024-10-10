#pragma once

#include <memory>
#include <vector>
#include "core/Math.h"

struct GeometryData;

class Mesh {
	friend class MeshLoader;

public:
	using PTR = std::shared_ptr<Mesh>;

	struct MeshNode {
		std::vector<MeshNode> children;
		std::vector<std::shared_ptr<GeometryData>> geoms;

		// TODO: replace with Transform struct when ready
		Math::Vector3 pos;
		Math::Quaternion rot;
		Math::Vector3 scale;
	};

	~Mesh();

	const MeshNode& GetRoot() const { return root; }

protected:
	MeshNode root;
};
