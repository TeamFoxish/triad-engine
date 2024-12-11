#include "Shader.h"

#include "logs/Logs.h"

#include "RenderContext.h"

#include <codecvt>

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


static std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.to_bytes(wstr);
}


Shader::Shader(
	const std::wstring& path, CreationFlags flags, ID3D11Device* device,
	const D3D11_INPUT_ELEMENT_DESC* inpDescs, int inpDescsNum,
	const D3D_SHADER_MACRO* macros, int macrosCount) 
{
	//D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	if (flags & VERTEX_SH) {
		ID3DBlob* vertexBC = nullptr;
		ID3DBlob* errorCode = nullptr;
		auto res = D3DCompileFromFile(
			path.c_str(),
			nullptr /*macros*/,
			nullptr /*include*/,
			"VSMain",
			"vs_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // TODO: add debug flags only in debug
			0,
			&vertexBC,
			&errorCode);

		if (FAILED(res)) {
			if (errorCode) {
				char* compileErrors = (char*)(errorCode->GetBufferPointer());
				LOG_ERROR("failed to compile vertex shader {}: {}", ws2s(path), compileErrors);
			} else {
				LOG_ERROR("failed to find vertex shader file {}", ws2s(path));
			}
		} else {
			device->CreateVertexShader(
				vertexBC->GetBufferPointer(),
				vertexBC->GetBufferSize(),
				nullptr, &vertShader);
			device->CreateInputLayout(
				inpDescs,
				inpDescsNum,
				vertexBC->GetBufferPointer(),
				vertexBC->GetBufferSize(),
				&layout);
		}
	}
	if (flags & PIXEL_SH) {
		ID3DBlob* pixelBC = nullptr;
		ID3DBlob* errorCode = nullptr;
		auto res = D3DCompileFromFile(
			path.c_str(), 
			macros /*macros*/, 
			nullptr /*include*/, 
			"PSMain", 
			"ps_5_0", 
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 
			0, 
			&pixelBC, 
			&errorCode);

		if (FAILED(res)) {
			if (errorCode) {
				char* compileErrors = (char*)(errorCode->GetBufferPointer());
				LOG_ERROR("failed to compile pixel shader {}: {}", ws2s(path), compileErrors);
			} else {
				LOG_ERROR("failed to find pixel shader file {}", ws2s(path));
			}
		} else {
			device->CreatePixelShader(
				pixelBC->GetBufferPointer(),
				pixelBC->GetBufferSize(),
				nullptr, &pixShader);
		}
	}
}


Shader::Shader(
	const std::wstring& path, CreationFlags flags, ID3D11Device* device,
	const D3D11_INPUT_ELEMENT_DESC* inpDescs, int inpDescsNum,
	const D3D11_BUFFER_DESC* vsCBDescs, int vsCBDescsNum,
	const D3D11_BUFFER_DESC* psCBDescs, int psCBDescsNum,
	const D3D_SHADER_MACRO* macros, int macrosCount) 
	: Shader(path, flags, device, inpDescs, inpDescsNum, macros, macrosCount)
{
	InitCB(device, vsCBDescs, vsCBDescsNum, psCBDescs, psCBDescsNum);
}

void Shader::Activate(RenderContext& ctx, const Shader::PTR& thisShader)
{
	if (layout) {
		ctx->IASetInputLayout(layout);
	}
	ctx->VSSetShader(vertShader, nullptr, 0);
	if (vertShader && !cbVS.empty()) {
		ctx->VSSetConstantBuffers(0, (uint32_t)cbVS.size(), &cbVS.front());
	}
	ctx->PSSetShader(pixShader, nullptr, 0);
	if (pixShader && !cbPS.empty()) {
		ctx->PSSetConstantBuffers(0, (uint32_t)cbPS.size(), &cbPS.front());
	}
	ctx.activeShader = thisShader;
}

void Shader::InitCB(
	ID3D11Device* device,
	const D3D11_BUFFER_DESC* vsCBDescs, int vsCBDescsNum, 
	const D3D11_BUFFER_DESC* psCBDescs, int psCBDescsNum)
{
	if (vertShader && vsCBDescs && vsCBDescsNum > 0) {
		cbVS.resize(vsCBDescsNum);
		cbVSSizes.resize(vsCBDescsNum);
		for (int i = 0; i < vsCBDescsNum; ++i) {
			device->CreateBuffer(&vsCBDescs[i], nullptr, &cbVS[i]);
			cbVSSizes[i] = vsCBDescs[i].ByteWidth;
		}
	}
	if (pixShader && psCBDescs && psCBDescsNum > 0) {
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
