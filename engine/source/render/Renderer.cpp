#include "Renderer.h"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"
#include "FrameGraphResources.h"

#include "os/Window.h"
#include "RenderContext.h"
#include "Renderable.h"
#include "RenderSystem.h"
#include "GeometryData.h"
#include "mesh/MeshLoader.h"
#include "TextureLoader.h"
#include "Shader.h"

#include "logs/Logs.h"

#include "input/InputDevice.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <chrono>
#include <algorithm>

#ifdef EDITOR
#include "editor/ui_debug/UIDebug.h"
#include "passes/DbgDrawPass.h"
#endif

#ifdef _WIN32
#include "os/wnd.h"
#endif

//#define TRIAD_LOG_FRAMEGRAPH
#ifdef TRIAD_LOG_FRAMEGRAPH
#include <fstream>
#endif

bool Renderer::Initialize(Window* _window)
{
	using namespace Triad::Render::Api;
	window = context.window = _window;

	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(window->GetWidth());
	viewport.Height = static_cast<float>(window->GetHeigth());
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;
	context.viewport = viewport;

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
		&context.swapChain,
		&context.device,
		nullptr,
		&context.context);

	if (FAILED(res))
	{
		// Well, that was unexpected
		__debugbreak();
	}
	{
		texToBackBuffShader = std::make_shared<Shader>(L"shaders/texture_to_backbuffer.hlsl", (Shader::CreationFlags)(Shader::VERTEX_SH | Shader::PIXEL_SH), context.device, nullptr, 0, nullptr, 0);
	}

	//ID3D11Texture2D* backTex;
	//res = context.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);	// __uuidof(ID3D11Texture2D)
	//res = context.device->CreateRenderTargetView(backTex, nullptr, &rtv);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	//rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = true;

	res = context.device->CreateRasterizerState(&rastDesc, &context.masterRastState);
	//context->RSSetState(rastState);

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
	context.device->CreateSamplerState(&samplerDesc, &context.masterSamplerState);
	//context->PSSetSamplers(0, 1, &samplerState);

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
	context.device->CreateDepthStencilState(&dsDesc, &pDSState);

	utils = std::make_unique<RenderUtils>();

	deferredRenderer = std::make_unique<DeferredRenderer>(context);
	uiPass = std::make_unique<UIPass>(context);

	return true;
}

void Renderer::Shutdown()
{
	deferredRenderer.reset();

	utils.reset();

	// TODO: term render context
}

void Renderer::Draw()
{
	context.ClearState();
	if (!gRenderSys->cameraManager.HasActiveCamera()) {
		LOG_WARN("unable to draw a frame since no active camera was set");
		return;
	}
	TestFrameGraph();
	context.ResetFrame();
}

void Renderer::EndFrame()
{
	context.swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
}

void Renderer::SetClearColor(const float* color)
{
	memcpy(clearColor, color, sizeof(float) * 4);
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

void Renderer::TestFrameGraph()
{
    using namespace Triad::Render::Api;
	FrameGraph fg;
    FrameGraphBlackboard bboard;

	deferredRenderer->Draw(context, fg, bboard);
#ifdef EDITOR
	if (!UIDebug::start_simulation) {
		DbgDrawPass::AddDbgDrawPass(context, fg, bboard);
	}
#endif
	uiPass->AddUIPass(context, fg, bboard);

	struct CompositionPassData {
		FrameGraphResource target;
	};
    const auto& sceneColor = bboard.get<DeferredLightingPass::DeferredLightingPassData>();
    static Texture2D* backTex = nullptr;
	if (!backTex) {
		context.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex); // TODO: cache backTex pointer??
	}
    FrameGraphResource backBuff = fg.import("BackBuffer", FrameGraphResources::FGTexture::Desc{}, FrameGraphResources::FGTexture{backTex});
    bboard.add<CompositionPassData>() = fg.addCallbackPass<CompositionPassData>("CompositionPass",
        [&](FrameGraph::Builder& builder, CompositionPassData& data) {
            builder.read(sceneColor.sceneColor);
            data.target = builder.write(backBuff);
        },
        [=, this](const CompositionPassData& data, FrameGraphPassResources& resources, void*) {			
			context.ClearState();
			context.activeDepthBuffuer = nullptr;

			context->RSSetState(context.masterRastState);
			context->PSSetSamplers(0, 1, &context.masterSamplerState);
			context->RSSetViewports(1, context.viewport.Get11());
			context->OMSetDepthStencilState(pDSState, 1);

            auto& texture = resources.get<FrameGraphResources::FGTexture>(sceneColor.sceneColor);
            D3D11_SHADER_RESOURCE_VIEW_DESC srtDesc = {};
            srtDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            srtDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srtDesc.Texture2D.MipLevels = 1;
			colorPassSrt = texture.BindRead(context, srtDesc, 0);

            auto& targetTex = resources.get<FrameGraphResources::FGTexture>(backBuff);
			if (context.backbuffResized) {
				mainRtv = nullptr;
				targetTex.Destroy();
				ResizeBackBuff();
				context.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);
				targetTex.tex = backTex;
			}
            RenderTarget* rtv = targetTex.BindWrite(context, nullptr, 0);
			mainRtv = rtv;
			context->ClearRenderTargetView(rtv, clearColor);

			context.TEMP_UpdateRenderTargetsNum();
			context->OMSetRenderTargets(context.activeRenderTargetNum, context.activeRenderTargets, context.activeDepthBuffuer);

#ifndef EDITOR
			// note: game viewport is drawn by imgui editor
			DrawScreenQuad();
#endif // !EDITOR
        }
    );

    fg.compile();
    fg.execute(&context, &context);

#ifdef TRIAD_LOG_FRAMEGRAPH
	struct GraphLogger {
	public:
		GraphLogger(const FrameGraph& fg) {
			std::ofstream f("fg.dot");
			f << fg;
		}
	};
	static GraphLogger _gLogger{fg};
#endif
}

void Renderer::DrawScreenQuad()
{
	texToBackBuffShader->Activate(context, texToBackBuffShader);
	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	context->Draw(4, 0);
}

void Renderer::ResizeBackBuff()
{
	context->OMSetRenderTargets(0, nullptr, nullptr);
	for (uint32_t i = 0; i < context.activeRenderTargetNum; ++i) {
		context.activeRenderTargets[i] = nullptr;
	}
	context.activeDepthBuffuer = nullptr;
	context.swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
}
