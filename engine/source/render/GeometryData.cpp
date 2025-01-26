#include "GeometryData.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

GeometryData::GeometryData(
	ID3D11Device* device,
	const void* vertData, int vertSize, 
	const uint32_t* idxData, int idxSize, 
	const std::vector<uint32_t> strides,
	const std::vector<uint32_t> offsets)
	: strides(strides), 
	  offsets(offsets)
{
	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = vertSize;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertData;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	device->CreateBuffer(&vertexBufDesc, &vertexData, &vertexBuf);

	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = idxSize;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = idxData;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	device->CreateBuffer(&indexBufDesc, &indexData, &indexBuf);
	idxNum = idxSize / sizeof(uint32_t);

#ifdef EDITOR
	const int stride = strides[0];
	if (stride == 0) {
		// log error
		return;
	}
	assert(vertSize % stride == 0);
	const int vNum = vertSize / stride;
	const char* srcVerts = static_cast<const char*>(vertData);
	vertices.resize(vNum * 3);
	for (int i = 0; i < vNum; ++i) {
		const int offsetDst = 3 * i;
		const int offsetSrc = stride * i;
		memcpy(&vertices[offsetDst], &srcVerts[offsetSrc], sizeof(float) * 3);
	}
	assert(idxSize % sizeof(uint32_t) == 0);
	const int idxNum = idxSize / sizeof(uint32_t);
	indices.assign(idxData, idxData + idxNum);
#endif //EDITOR
}

void GeometryData::Activate(ID3D11DeviceContext* context) const
{
	context->IASetPrimitiveTopology(primitiveType);
	context->IASetVertexBuffers(0, 1, &vertexBuf, &strides.front(), &offsets.front());
	context->IASetIndexBuffer(indexBuf, DXGI_FORMAT_R32_UINT, 0);
}
