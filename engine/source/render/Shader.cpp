#include "Shader.h"

#include <iostream>

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


Shader::Shader(
	const std::wstring& path, ID3D11Device* device,
	const D3D11_INPUT_ELEMENT_DESC* inpDescs, int inpDescsNum,
	const D3D_SHADER_MACRO* macros, int macrosCount) 
{
	ID3DBlob* vertexBC = nullptr;
	ID3DBlob* errorVertexCode = nullptr;
	auto res = D3DCompileFromFile(path.c_str(),
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexBC,
		&errorVertexCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorVertexCode) {
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			//MessageBox(hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
		}

		//return 0;
	}

	//D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* pixelBC;
	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(path.c_str(), macros /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelBC, &errorPixelCode);

	device->CreateVertexShader(
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		nullptr, &vertShader);

	device->CreatePixelShader(
		pixelBC->GetBufferPointer(),
		pixelBC->GetBufferSize(),
		nullptr, &pixShader);

	device->CreateInputLayout(
		inpDescs,
		inpDescsNum,
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		&layout);
}


Shader::Shader(
	const std::wstring& path, ID3D11Device* device,
	const D3D11_INPUT_ELEMENT_DESC* inpDescs, int inpDescsNum,
	const D3D11_BUFFER_DESC* vsCBDescs, int vsCBDescsNum,
	const D3D11_BUFFER_DESC* psCBDescs, int psCBDescsNum,
	const D3D_SHADER_MACRO* macros, int macrosCount) 
	: Shader(path, device, inpDescs, inpDescsNum, macros, macrosCount)
{
	InitCB(device, vsCBDescs, vsCBDescsNum, psCBDescs, psCBDescsNum);
}

void Shader::Activate(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(layout);
	context->VSSetShader(vertShader, nullptr, 0);
	if (!cbVS.empty()) {
		context->VSSetConstantBuffers(0, (uint32_t)cbVS.size(), &cbVS.front());
	}
	context->PSSetShader(pixShader, nullptr, 0);
	if (!cbPS.empty()) {
		context->PSSetConstantBuffers(0, (uint32_t)cbPS.size(), &cbPS.front());
	}
}

void Shader::InitCB(
	ID3D11Device* device,
	const D3D11_BUFFER_DESC* vsCBDescs, int vsCBDescsNum, 
	const D3D11_BUFFER_DESC* psCBDescs, int psCBDescsNum)
{
	if (vsCBDescs && vsCBDescsNum > 0) {
		cbVS.resize(vsCBDescsNum);
		cbVSSizes.resize(vsCBDescsNum);
		for (int i = 0; i < vsCBDescsNum; ++i) {
			device->CreateBuffer(&vsCBDescs[i], nullptr, &cbVS[i]);
			cbVSSizes[i] = vsCBDescs[i].ByteWidth;
		}
	}
	if (psCBDescs && psCBDescsNum > 0) {
		cbPS.resize(psCBDescsNum);
		cbPSSizes.resize(psCBDescsNum);
		for (int i = 0; i < psCBDescsNum; ++i) {
			device->CreateBuffer(&psCBDescs[i], nullptr, &cbPS[i]);
			cbPSSizes[i] = psCBDescs[i].ByteWidth;
		}
	}
}

void Shader::SetCBVS(ID3D11DeviceContext* context, int slot, void* data)
{
	//context->UpdateSubresource(cbVS[slot], 0, nullptr, data, 0, 0);
	D3D11_MAPPED_SUBRESOURCE subres;
	context->Map(cbVS[slot], 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, data, cbVSSizes[slot]);
	context->Unmap(cbVS[slot], 0);
}

void Shader::SetCBPS(ID3D11DeviceContext* context, int slot, void* data)
{
	//context->UpdateSubresource(cbPS[slot], 0, nullptr, data, 0, 0);
	D3D11_MAPPED_SUBRESOURCE subres;
	context->Map(cbPS[slot], 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
	memcpy(subres.pData, data, cbPSSizes[slot]);
	context->Unmap(cbPS[slot], 0);
}
