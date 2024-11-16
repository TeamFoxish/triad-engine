#include "D3D11Helpers.h"

#include "render/RenderContext.h"

#include <d3dcompiler.h>

namespace Triad::Render {
namespace Api {

Texture2D* CreateTexture(const RenderContext& ctx, TextureDesc desc)
{
	assert(desc.fitToViewport || desc.texDesc.Width > 0 && desc.texDesc.Width > 0);
	if (desc.fitToViewport) {
		// used for full screen textures
		desc.texDesc.Width = (uint32_t)ctx.viewport.width;
		desc.texDesc.Height = (uint32_t)ctx.viewport.height;
	}
	
	ID3D11Texture2D* tex = nullptr;
	if (FAILED(ctx.device->CreateTexture2D(&desc.texDesc, NULL, &tex))) {
		assert(false);
		return nullptr;
	}
	return tex;
}

ShaderResource* CreateShaderResource(const RenderContext& ctx, Texture2D* tex, const ShaderResourceDesc& desc)
{
	ShaderResource* res;
	if (FAILED(ctx.device->CreateShaderResourceView(tex, &desc, &res))) {
		assert(false);
		return nullptr;
	}
	return res;
}

RenderTarget* CreateRenderTarget(const RenderContext& ctx, Texture2D* tex, const RenderTargetDesc* desc)
{
	RenderTarget* res;
	if (FAILED(ctx.device->CreateRenderTargetView(tex, desc, &res))) {
		assert(false);
		return nullptr;
	}
	return res;
}

DepthBuffer* CreateDepthBuffer(const RenderContext& ctx, Texture2D* tex, const DepthBufferDesc& desc)
{
	DepthBuffer* res;
	if (FAILED(ctx.device->CreateDepthStencilView(tex, &desc, &res))) {
		assert(false);
		return nullptr;
	}
	return res;
}

}
}
