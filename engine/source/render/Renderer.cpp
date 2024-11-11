#include "Renderer.h"

#include "os/Window.h"
#include "GeometryData.h"
#include "MeshLoader.h"
#include "TextureLoader.h"
#include "Lights.h"
#include "Shader.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <chrono>
#include <algorithm>

#ifdef _WIN32
#include "os/wnd.h"
#endif

#include "editor/ui_debug/UIDebug.h"

bool Renderer::Initialize(Window* _window)
{
	window = _window;
	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Width = window->GetWidth();
	swapDesc.BufferDesc.Height = window->GetHeigth();
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
#ifdef _WIN32
	swapDesc.OutputWindow = wndGetHWND(window);
#elif
	static_assert(false, "only windows support");
#endif
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	auto res = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&swapChain,
		&device,
		nullptr,
		&context);

	if (FAILED(res))
	{
		// Well, that was unexpected
		__debugbreak();
	}
	{
		texToBackBuffShader = new Shader(L"shaders/texture_to_backbuffer.hlsl", device, nullptr, 0, nullptr, 0);
	}
	{   // SCREEN_TEX
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = window->GetWidth();
		texDesc.Height = window->GetHeigth();
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R32G32B32A32_TYPELESS;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.ArraySize = 1;

		ID3D11Texture2D* tex;
		if (FAILED(device->CreateTexture2D(&texDesc, NULL, &tex))) {
			assert(false);
			return false;
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		if (FAILED(device->CreateRenderTargetView(tex, &rtvDesc, &colorPassRtv))) {
			assert(false);
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srtDesc = {};
		srtDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srtDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srtDesc.Texture2D.MipLevels = 1;
		if (FAILED(device->CreateShaderResourceView(tex, &srtDesc, &colorPassSrt))) {
			assert(false);
			return false;
		}

		tex->Release();
	}

	ID3D11Texture2D* backTex;
	res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);	// __uuidof(ID3D11Texture2D)
	res = device->CreateRenderTargetView(backTex, nullptr, &rtv);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = device->CreateRasterizerState(&rastDesc, &rastState);
	context->RSSetState(rastState);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	device->CreateSamplerState(&samplerDesc, &samplerState);
	context->PSSetSamplers(0, 1, &samplerState);

	D3D11_TEXTURE2D_DESC depthTextureDesc = {};
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
	depthTextureDesc.Width = window->GetWidth();
	depthTextureDesc.Height = window->GetHeigth();
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.SampleDesc.Quality = 0;

	ID3D11Texture2D* dsTexutre;
	if (FAILED(device->CreateTexture2D(&depthTextureDesc, NULL, &dsTexutre))) {
		exit(-1);
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = depthTextureDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	device->CreateDepthStencilView(dsTexutre, &dsvDesc, &depthBuffer);
	dsTexutre->Release();

	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	device->CreateDepthStencilState(&dsDesc, &pDSState);

	utils = std::make_unique<RenderUtils>();

	return true;
}

void Renderer::Shutdown()
{
	utils.reset();

	delete texToBackBuffShader;
}

void Renderer::Draw()
{
	context->ClearState();
	context->RSSetState(rastState);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(window->GetWidth());
	viewport.Height = static_cast<float>(window->GetHeigth());
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	context->RSSetViewports(1, &viewport);

#ifdef EDITOR
	// draw to texture pass
	context->OMSetRenderTargets(1, &colorPassRtv, depthBuffer);
#else
	context->OMSetRenderTargets(1, &rtv, depthBuffer);
#endif // EDITOR

	context->OMSetDepthStencilState(pDSState, 1);

#ifdef EDITOR
	context->ClearRenderTargetView(colorPassRtv, clearColor);
#else
	context->ClearRenderTargetView(rtv, clearColor);
#endif // EDITOR

	context->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->PSSetSamplers(0, 1, &samplerState);

	for (DrawComponent* comp : components) {
		comp->Draw(this);
	}

#ifdef EDITOR
	// draw to screen pass
	context->OMSetRenderTargets(1, &rtv, nullptr);

	context->OMSetDepthStencilState(nullptr, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	texToBackBuffShader->Activate(context);
	context->IASetInputLayout(nullptr);
	context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	context->PSSetShaderResources(0, 1, &colorPassSrt);

	context->Draw(4, 0);
#endif // EDITOR
}

void Renderer::EndFrame()
{
	swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
}

void Renderer::SetClearColor(const float* color)
{
	memcpy(clearColor, color, sizeof(float) * 4);
}

void Renderer::PopulateLightsBuffer(DefaultMeshMaterial::CBPS& buffer) const
{
	for (const Light* light : lightSources) {
		light->UpdateBuffer(buffer);
	}
}

Mesh::PTR Renderer::GetMesh(const std::string& path)
{
	auto iter = meshes.find(path);
	if (iter != meshes.end()) {
		return iter->second;
	}
	Mesh::PTR mesh;
	if (!MeshLoader::LoadMesh(path, this, mesh)) {
		return nullptr;
	}
	meshes[path] = mesh;

	return meshes[path];
}

ID3D11ShaderResourceView* Renderer::GetTexture(const std::wstring& path)
{
	// TODO: add texture unload if not used
	auto iter = textures.find(path);
	if (iter != textures.end()) {
		return iter->second;
	}
	ID3D11ShaderResourceView* res;
	if (!TextureLoader::LoadTexture(path, this, &res)) {
		return nullptr;
	}
	textures[path] = res;

	return res;
}

void Renderer::AddComponent(DrawComponent* comp)
{
	components.push_back(comp);
}

void Renderer::RemoveComponent(DrawComponent* comp)
{
	auto iter = std::find(components.begin(), components.end(), comp);
	if (iter != components.end())
	{
		std::iter_swap(iter, components.end() - 1);
		components.pop_back();
		return;
	}
}

void Renderer::AddLight(Light* light)
{
	lightSources.push_back(light);
}

void Renderer::RemoveLight(Light* light)
{
	auto iter = std::find(lightSources.begin(), lightSources.end(), light);
	if (iter != lightSources.end())
	{
		std::iter_swap(iter, lightSources.end() - 1);
		lightSources.pop_back();
		return;
	}
}
