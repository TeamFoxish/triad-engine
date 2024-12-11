#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"

#include <assimp/defs.h>
#include <string>
#include "Mesh.h"

class Renderer;

template<typename>
class aiMatrix4x4t;
typedef class aiMatrix4x4t<ai_real> aiMatrix4x4;

struct aiScene;
struct aiNode;

class MeshLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, MeshLoader> {
public:
	MeshLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override;

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<MeshLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "mesh";
	}

	static bool LoadMesh(const std::string& path, Renderer* renderer, std::shared_ptr<Mesh>& outMesh);

protected:
	static void CopyNodesWithMeshes(const aiScene* scene, aiNode* node, Mesh& targetParent, Renderer* renderer, const aiMatrix4x4& accTransform);

	static void AddMesh(const aiScene* scene, int meshIdx, Renderer* renderer, Mesh::MeshNode& target);
};
