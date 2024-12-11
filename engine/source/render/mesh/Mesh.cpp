#include "Mesh.h"

Mesh::~Mesh()
{
	// TODO: delete mesh data
}

Mesh::PTR Mesh::CreateFromGeom(const std::shared_ptr<GeometryData>& geom)
{
	Mesh::PTR res = std::make_shared<Mesh>();
	MeshNode& node = res->nodes.emplace_back();
	node.geoms.push_back(geom);
	return res;
}
