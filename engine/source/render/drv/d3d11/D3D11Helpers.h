#pragma once

#include <d3d.h>
#include <d3d11.h>
#include <BufferHelpers.h>
#include <CommonStates.h>

struct RenderContext;

namespace Triad::Render {
namespace Helpers 
{
	using namespace DirectX;
}

namespace Api {
	struct TextureDesc {
		D3D11_TEXTURE2D_DESC texDesc = {};
		bool fitToViewport = false;
	};
	using Texture2D = ID3D11Texture2D;
	Texture2D* CreateTexture(const RenderContext& ctx, TextureDesc desc);

	using ShaderResourceDesc = D3D11_SHADER_RESOURCE_VIEW_DESC;
	using ShaderResource = ID3D11ShaderResourceView;
	ShaderResource* CreateShaderResource(const RenderContext& ctx, Texture2D* tex, const ShaderResourceDesc& desc);

	using RenderTargetDesc = D3D11_RENDER_TARGET_VIEW_DESC;
	using RenderTarget = ID3D11RenderTargetView;
	RenderTarget* CreateRenderTarget(const RenderContext& ctx, Texture2D* tex, const RenderTargetDesc* desc);

	using DepthBufferDesc = D3D11_DEPTH_STENCIL_VIEW_DESC;
	using DepthBuffer = ID3D11DepthStencilView;
	DepthBuffer* CreateDepthBuffer(const RenderContext& ctx, Texture2D* tex, const DepthBufferDesc& desc);
}
}
