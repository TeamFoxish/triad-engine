#include "RenderUtils.h"

#include "Renderer.h"
#include "GeometryData.h"
#include "Shader.h"
#include "SphereCreator.h"
#include "Material.h"

#include <cassert>
#include <iostream>

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <d3dcommon.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

RenderUtils::~RenderUtils()
{
	// TODO: terminate all geom data
}

std::shared_ptr<Shader> RenderUtils::GetQuadShader(Renderer* renderer)
{
	if (quadShader) {
		return quadShader;
	}

	quadShader = CreateQuadShader(renderer);
	return quadShader;
}

GeometryData::PTR RenderUtils::GetQuadGeom(Renderer* renderer)
{
    if (quadGeom) {
        return quadGeom;
    }
	
	quadGeom = CreateQuadGeom(renderer);
	return quadGeom;
}

std::shared_ptr<GeometryData> RenderUtils::GetTriGeom(Renderer* renderer, const void* vertFloats, int vertsSize)
{
	const uint32_t indices[] = { 0,1,2 };
	return std::make_shared<GeometryData>(
		renderer->GetDevice(),
		vertFloats, vertsSize,
		indices, (int)(sizeof(uint32_t) * std::size(indices)),
		std::vector<uint32_t>{ 32 }, std::vector<uint32_t>{0});
}

std::shared_ptr<Shader> RenderUtils::GetAdvMeshShader(Renderer* renderer, int cbVSSize, int cbPSSize)
{
	if (meshAdvShader) {
		return meshAdvShader;
	}

	meshAdvShader = CreateAdvMeshShader(renderer, cbVSSize, cbPSSize);
	return meshAdvShader;
}

std::shared_ptr<GeometryData> RenderUtils::GetCubeGeom(Renderer* renderer)
{
	if (cubeGeom) {
		return cubeGeom;
	}

	cubeGeom = CreateCubeGeom(renderer);
	return cubeGeom;
}

std::shared_ptr<GeometryData> RenderUtils::GetSphereGeom(Renderer* renderer)
{
	if (sphereGeom) {
		return sphereGeom;
	}

	sphereGeom = CreateSphereGeom(renderer);
	return sphereGeom;
}

std::shared_ptr<Shader> RenderUtils::CreateQuadShader(Renderer* renderer)
{
	const D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};
	assert(sizeof(QuadCBVS) % 16 == 0);
	const D3D11_BUFFER_DESC cbVSDescs[] = {
		{
			sizeof(QuadCBVS),			// UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
			D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
			0,							// UINT MiscFlags;
			0,							// UINT StructureByteStride;
		}
	};
	assert(sizeof(QuadCBPS) % 16 == 0);
	const D3D11_BUFFER_DESC cbPSDescs[] = {
		{
			sizeof(QuadCBPS),			// UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
			D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
			0,							// UINT MiscFlags;
			0,							// UINT StructureByteStride;
		}
	};

	return std::make_shared<Shader>(L"shaders/default_shader.hlsl", renderer->GetDevice(), inputElements, 1, cbVSDescs, 1, cbPSDescs, 1);
}

GeometryData::PTR RenderUtils::CreateQuadGeom(Renderer* renderer)
{
	const DirectX::XMFLOAT4 points[] = {
		DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),	
		DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f),
		DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f),	
		DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f),	
	};
	const uint32_t indices[] = { 0,1,2, 1,0,3 };
	return std::make_shared<GeometryData>(
		renderer->GetDevice(),
		points, (int)(sizeof(DirectX::XMFLOAT4) * std::size(points)),
		indices, (int)(sizeof(uint32_t) * std::size(indices)),
		std::vector<uint32_t>{ 16 }, std::vector<uint32_t>{0});
}

std::shared_ptr<Shader> RenderUtils::CreateAdvMeshShader(Renderer* renderer, int cbVSSize, int cbPSSize)
{
	const D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		D3D11_INPUT_ELEMENT_DESC {
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		D3D11_INPUT_ELEMENT_DESC {
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
	};
	//assert(sizeof(MeshCBVS) % 16 == 0);
	const D3D11_BUFFER_DESC cbVSDescs[] = {
		{
			(uint32_t)cbVSSize,			// UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
			D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
			0,							// UINT MiscFlags;
			0,							// UINT StructureByteStride;
		}
	};
	//assert(sizeof(MeshCBPS) % 16 == 0);
	const D3D11_BUFFER_DESC cbPSDescs[] = {
		{
			(uint32_t)cbPSSize,			// UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
			D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
			0,							// UINT MiscFlags;
			0,							// UINT StructureByteStride;
		}
	};

	return std::make_shared<Shader>(L"shaders/advanced_mesh_shader.hlsl", renderer->GetDevice(), inputElements, (int)std::size(inputElements), cbVSDescs, 1, cbPSDescs, 1);
}

std::shared_ptr<GeometryData> RenderUtils::CreateCubeGeom(Renderer* renderer)
{
	const DirectX::XMFLOAT4 points[] = {
		DirectX::XMFLOAT4(-0.5,-0.5,-0.5, 1.0f),
		DirectX::XMFLOAT4(0.5,-0.5,-0.5 , 1.0f),
		DirectX::XMFLOAT4(-0.5,0.5,-0.5 , 1.0f),
		DirectX::XMFLOAT4(0.5,0.5,-0.5  , 1.0f),
		DirectX::XMFLOAT4(-0.5,0.5,0.5  , 1.0f),
		DirectX::XMFLOAT4(0.5,0.5,0.5   , 1.0f),
		DirectX::XMFLOAT4(-0.5,-0.5,0.5 , 1.0f),
		DirectX::XMFLOAT4(0.5,-0.5,0.5  , 1.0f),
		DirectX::XMFLOAT4(-0.5,0.5,-0.5 , 1.0f),
		DirectX::XMFLOAT4(0.5,-0.5,-0.5 , 1.0f),
		DirectX::XMFLOAT4(-0.5,0.5,-0.5 , 1.0f),
		DirectX::XMFLOAT4(0.5,0.5,-0.5  , 1.0f),
		DirectX::XMFLOAT4(-0.5,0.5,0.5  , 1.0f),
		DirectX::XMFLOAT4(-0.5,0.5,0.5  , 1.0f),
		DirectX::XMFLOAT4(0.5,0.5,0.5   , 1.0f),
		DirectX::XMFLOAT4(-0.5,-0.5,0.5 , 1.0f),
		DirectX::XMFLOAT4(-0.5,-0.5,0.5 , 1.0f),
		DirectX::XMFLOAT4(0.5,-0.5,0.5  , 1.0f),
		DirectX::XMFLOAT4(-0.5,-0.5,-0.5, 1.0f),
		DirectX::XMFLOAT4(0.5,-0.5,-0.5 , 1.0f),
		DirectX::XMFLOAT4(0.5,-0.5,-0.5 , 1.0f),
		DirectX::XMFLOAT4(0.5,-0.5,0.5  , 1.0f),
		DirectX::XMFLOAT4(0.5,0.5,-0.5  , 1.0f),
		DirectX::XMFLOAT4(0.5,0.5,0.5   , 1.0f),
		DirectX::XMFLOAT4(-0.5,-0.5,0.5 , 1.0f),
		DirectX::XMFLOAT4(-0.5,-0.5,-0.5, 1.0f),
		DirectX::XMFLOAT4(-0.5,0.5,0.5  , 1.0f),
		DirectX::XMFLOAT4(-0.5,0.5,-0.5 , 1.0f),
	};

	const uint32_t indices[] = { 
		2,1,0	,
		3,9,8	,
		4,11,10	,
		5,11,12	,
		6,14,13	,
		7,14,15	,
		18,17,16,
		19,17,18,
		22,21,20,
		23,21,22,
		26,25,24,
		27,25,26,
	};
	return std::make_shared<GeometryData>(
		renderer->GetDevice(),
		points, (int)(sizeof(DirectX::XMFLOAT4) * std::size(points)),
		indices, (int)(sizeof(uint32_t) * std::size(indices)),
		std::vector<uint32_t>{ 16 }, std::vector<uint32_t>{0});
}

std::shared_ptr<GeometryData> RenderUtils::CreateSphereGeom(Renderer* renderer)
{
	IcoSphereCreator sphereCreator;
	IcoSphereCreator::SphereGeom geom = sphereCreator.Create(2);
	return std::make_shared<GeometryData>(
		renderer->GetDevice(),
		static_cast<void*>(geom.geometry.data()), (int)(sizeof(IcoSphereCreator::VertData) * geom.geometry.size()),
		reinterpret_cast<uint32_t*>(geom.indices.data()), (int)(sizeof(uint32_t) * geom.indices.size() * 3),
		std::vector<uint32_t>{ 32 }, std::vector<uint32_t>{0});
}
