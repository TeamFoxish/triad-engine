#pragma once

#include <d3dcommon.h>
#include <memory>
#include <vector>

struct GeometryData {
	using PTR = std::shared_ptr<GeometryData>;

	GeometryData(
		struct ID3D11Device* device,
		const void* vertData, int vertSize, 
		const uint32_t* idxData, int idxSize, 
		const std::vector<uint32_t> strides, 
		const std::vector<uint32_t> offsets);

	void Activate(struct ID3D11DeviceContext* context) const;

	struct ID3D11Buffer* vertexBuf;
	struct ID3D11Buffer* indexBuf;
	D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	const std::vector<uint32_t> strides;
	const std::vector<uint32_t> offsets;

	int idxNum;

#ifdef EDITOR
	std::vector<float> vertices; // vec3
	std::vector<uint32_t> indices;
#endif //EDITOR
};
