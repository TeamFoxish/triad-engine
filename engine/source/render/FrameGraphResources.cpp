#include "FrameGraphResources.h"

#include "misc/Hash.hpp"
#include "RenderContext.h"

#include <memory>
#include <unordered_map>
#include <vector>

using FGTextureDesc = FrameGraphResources::FGTexture::Desc;

namespace std {
	template <> 
	struct hash<FGTextureDesc> {
		hash() = default;

		std::size_t operator()(const FGTextureDesc& desc) const noexcept {
			std::size_t h{0};
			hashCombine(h, desc.fitToViewport, desc.texDesc.Format, desc.texDesc.ArraySize, desc.texDesc.BindFlags, desc.texDesc.CPUAccessFlags, desc.texDesc.Height, desc.texDesc.MipLevels, desc.texDesc.MiscFlags, desc.texDesc.SampleDesc.Count, desc.texDesc.SampleDesc.Quality, desc.texDesc.Usage, desc.texDesc.Width);
			return h;
		}
	};
}

using namespace Triad::Render::Api;

class TransientResourcesStorage {
public:
	~TransientResourcesStorage() 
	{
		for (auto& [hash, texPool] : texturePool) {
			for (Texture2D* tex : texPool) {
				tex->Release();
			}
		}
		for (auto& [pTex, res] : textureShaderResources) {
			res->Release();
		}
		for (auto& [pTex, res] : textureRenderTargets) {
			res->Release();
		}
		for (auto& [pTex, res] : depthBuffers) {
			res->Release();
		}
		// TODO: term views
	}

	Texture2D* PullTexture(RenderContext& ctx, const FGTextureDesc& desc)
	{
		const auto h = std::hash<FGTextureDesc>{}(desc);
		const auto iter = texturePool.find(h);
		if (iter != texturePool.end() && !iter->second.empty()) {
			Texture2D* tex = iter->second.back();
			iter->second.pop_back();
			return tex;
		}
		return CreateTexture(ctx, desc);
	}

	void ReturnTexture(const FGTextureDesc& desc, Texture2D* tex)
	{
		const auto h = std::hash<FGTextureDesc>{}(desc);
		texturePool[h].push_back(tex);
	}

	ShaderResource* PullTextureShaderResource(RenderContext& ctx, const ShaderResourceDesc& desc, Texture2D* tex)
	{
		const auto iter = textureShaderResources.find(tex);
		if (iter != textureShaderResources.end()) {
			return iter->second;
		}
		ShaderResource* srt = CreateShaderResource(ctx, tex, desc);
		textureShaderResources[tex] = srt;
		return srt;
	}

	RenderTarget* PullTextureRenderTarget(RenderContext& ctx, const RenderTargetDesc* desc, Texture2D* tex)
	{
		const auto iter = textureRenderTargets.find(tex);
		if (iter != textureRenderTargets.end()) {
			return iter->second;
		}
		RenderTarget* rtv = CreateRenderTarget(ctx, tex, desc);
		textureRenderTargets[tex] = rtv;
		return rtv;
	}

	DepthBuffer* PullTextureDepthBuffer(RenderContext& ctx, const DepthBufferDesc& desc, Texture2D* tex)
	{
		const auto iter = depthBuffers.find(tex);
		if (iter != depthBuffers.end()) {
			return iter->second;
		}
		DepthBuffer* depthBuf = CreateDepthBuffer(ctx, tex, desc);
		depthBuffers[tex] = depthBuf;
		return depthBuf;
	}

	static std::unique_ptr<TransientResourcesStorage>& Instance()
	{
		static std::unique_ptr<TransientResourcesStorage> instance = std::make_unique<TransientResourcesStorage>();
		return instance;
	}

private:
	std::unordered_map<std::size_t, std::vector<Texture2D*>> texturePool;
	std::unordered_map<Texture2D*, ShaderResource*> textureShaderResources;
	std::unordered_map<Texture2D*, RenderTarget*> textureRenderTargets;
	std::unordered_map<Texture2D*, DepthBuffer*> depthBuffers;
};

void TermTransientStorage() 
{
	TransientResourcesStorage::Instance().reset();
}

void FrameGraphResources::FGTexture::create(const Desc& desc, void* allocator)
{
	RenderContext& ctx = *static_cast<RenderContext*>(allocator);
	tex = TransientResourcesStorage::Instance()->PullTexture(ctx, desc);
}

void FrameGraphResources::FGTexture::destroy(const Desc& desc, void* allocator)
{
	TransientResourcesStorage::Instance()->ReturnTexture(desc, std::move(tex));
}

ShaderResource* FrameGraphResources::FGTexture::BindRead(RenderContext& ctx, const ShaderResourceDesc& desc, uint32_t slot)
{
	ShaderResource* srt = TransientResourcesStorage::Instance()->PullTextureShaderResource(ctx, desc, tex);
	ctx.context->PSSetShaderResources(slot, 1, &srt);
	return srt;
}

RenderTarget* FrameGraphResources::FGTexture::BindWrite(RenderContext& ctx, const Triad::Render::Api::RenderTargetDesc* desc, uint32_t slot)
{
	assert(slot < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	RenderTarget* res = ctx.activeRenderTargets[slot] = TransientResourcesStorage::Instance()->PullTextureRenderTarget(ctx, desc, tex);
	// TODO: move away set render targets call to pipeline setup
	ctx.context->OMSetRenderTargets(1, ctx.activeRenderTargets, ctx.activeDepthBuffuer);
	return res;
}

auto FrameGraphResources::FGTexture::BindWrite(RenderContext& ctx, const Triad::Render::Api::DepthBufferDesc& desc) -> Triad::Render::Api::DepthBuffer*
{
	DepthBuffer* depthBuf = ctx.activeDepthBuffuer = TransientResourcesStorage::Instance()->PullTextureDepthBuffer(ctx, desc, tex);
	// TODO: move away set render targets call to pipeline setup
	ctx.context->OMSetRenderTargets(1, ctx.activeRenderTargets, depthBuf);
	return depthBuf;
}
