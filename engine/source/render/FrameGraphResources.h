#pragma once

#include "math/Math.h"
#include "Texture.h"

#include "drv/d3d11/D3D11Helpers.h"

struct FrameGraphResources {
	struct FGTexture {
		using Desc = Triad::Render::Api::TextureDesc;

		void create(const Desc& desc, void* allocator);
		void destroy(const Desc& desc, void* allocator);

		/*void preRead(const Desc& desc, uint32_t flags, void* context);
		void preWrite(const Desc& desc, uint32_t flags, void* context);*/

		auto BindRead(RenderContext& ctx, const Triad::Render::Api::ShaderResourceDesc& desc, uint32_t slot) -> Triad::Render::Api::ShaderResource*;
		auto BindWrite(RenderContext& ctx, const Triad::Render::Api::RenderTargetDesc* desc, uint32_t slot) -> Triad::Render::Api::RenderTarget*;
		auto BindWrite(RenderContext& ctx, const Triad::Render::Api::DepthBufferDesc& desc) -> Triad::Render::Api::DepthBuffer*;

		void Destroy();

		Triad::Render::Api::Texture2D* tex = nullptr;
	};
};

void TermTransientStorage();
