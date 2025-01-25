#ifdef EDITOR

#include "render/RenderContext.h"
#include "DbgDrawPass.h"
#include "DeferredLightingPass.h"
#include "GBufferPass.h"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"

#include "navigation/NavMeshSystem.h"
#include "physics/PhySystem.h"

DbgDrawPass::DbgDrawPass(RenderContext& ctx)
{
}

void DbgDrawPass::AddDbgDrawPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard)
{
	using namespace Triad::Render::Api;

	// TEMP
	if (!gNavigation->IsDebugDrawEnabled()) {
		return;
	}

	auto& targetPass = bboard.get<DeferredLightingPass::DeferredLightingPassData>();
	auto& gBuffer = bboard.get<GBufferPass::GBufferPassData>();
	bboard.add<DbgDrawPassData>() = fg.addCallbackPass<DbgDrawPassData>("DebugDraw",
		[&](FrameGraph::Builder& builder, DbgDrawPassData& data) {
			gBuffer.depthStencil = builder.write(gBuffer.depthStencil);
			targetPass.sceneColor = builder.write(targetPass.sceneColor);
		},
		[=, &ctx](const DbgDrawPassData& data, FrameGraphPassResources& resources, void*) {
			ctx.ClearState();

			ctx->RSSetState(ctx.masterRastState);
			ctx->PSSetSamplers(0, 1, &ctx.masterSamplerState);
			ctx->RSSetViewports(1, ctx.viewport.Get11());

			// depth
			auto& depthStencil = resources.get<FrameGraphResources::FGTexture>(gBuffer.depthStencil);
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			DepthBuffer* depthBuf = depthStencil.BindWrite(ctx, dsvDesc);

			auto& sceneColor = resources.get<FrameGraphResources::FGTexture>(targetPass.sceneColor);
			RenderTargetDesc rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			RenderTarget* rtv = sceneColor.BindWrite(ctx, &rtvDesc, 0);

			ctx.TEMP_UpdateRenderTargetsNum();
			ctx->OMSetRenderTargets(ctx.activeRenderTargetNum, ctx.activeRenderTargets, ctx.activeDepthBuffuer);
			DebugDraw(ctx);
		});
}

void DbgDrawPass::DebugDraw(RenderContext& ctx)
{
	gNavigation->DebugDraw();
	gPhySys->DebugDraw();
}

#endif // EDITOR
