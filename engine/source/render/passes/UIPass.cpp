#include "render/RenderContext.h" // place at the very top, so d3d11 is included first and Math::Viewport::Get11() is defined later

#include "UIPAss.h"
#include "DeferredLightingPass.h"
#include "render/ui/UIStorage.h"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"

UIPass::UIPass(RenderContext& ctx)
{
	_font = std::make_unique<DirectX::SpriteFont>(ctx.device, L"fonts\\DroidSans.spritefont");
	_batch = std::make_unique<DirectX::SpriteBatch>(ctx.context);
}

void UIPass::AddUIPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard)
{
	using namespace Triad::Render::Api;

	auto& targetPass = bboard.get<DeferredLightingPass::DeferredLightingPassData>();
    bboard.add<UIPassData>() = fg.addCallbackPass<UIPassData>("UIPass",
        [&](FrameGraph::Builder& builder, UIPassData& data) {
			targetPass.sceneColor = builder.write(targetPass.sceneColor);
        },
        [=, this, &ctx](const UIPassData& data, FrameGraphPassResources& resources, void*) {
			ctx.ClearState();

			// setup pipiline
			ctx->RSSetState(ctx.masterRastState);
			ctx->PSSetSamplers(0, 1, &ctx.masterSamplerState);
			ctx->RSSetViewports(1, ctx.viewport.Get11());

			auto& sceneColor = resources.get<FrameGraphResources::FGTexture>(targetPass.sceneColor);
			RenderTargetDesc rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			RenderTarget* rtv = sceneColor.BindWrite(ctx, &rtvDesc, 0);

			ctx.TEMP_UpdateRenderTargetsNum();
			ctx->OMSetRenderTargets(ctx.activeRenderTargetNum, ctx.activeRenderTargets, ctx.activeDepthBuffuer);
			
			// draw scene to gbuffer
			DrawUI(ctx);
        }
    );
}

void UIPass::DrawUI(RenderContext& ctx)
{
	_batch->Begin(DirectX::SpriteSortMode_BackToFront);
	for( const UIElement& element: UIStorage::Instance().GetStorage()) {
		DirectX::SimpleMath::Vector2 position = element.position;
		position.x *= ctx.viewport.width;
		position.y *= ctx.viewport.height;
		DirectX::SimpleMath::Vector2 origin = _font->MeasureString(element.text.c_str());
		origin = origin / 2.0f;
		_font->DrawString(
			_batch.get(),
			element.text.c_str(),
			position,
			element.color,
			0.f,
			origin,
			{1.f, 1.f},
			DirectX::DX11::SpriteEffects_None,
			element.depth
		);
	}
	_batch->End();
}