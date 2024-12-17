#include "render/RenderContext.h"
#include "DeferredLightingPass.h"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"

#include "render/light/LightsStorage.h"
#include "render/GeometryData.h"
#include "render/mesh/MeshRenderer.h"
#include "GBufferPass.h"

// TEMP
#include "runtime/EngineRuntime.h"
#include "game/Game.h"
#include "components/ThirdPersonCamera.h"

__declspec(align(16))
struct CBPS {
	Math::Matrix inverseViewMatr;
	Math::Matrix viewMatr;
	Math::Vector4 uCameraPos;
	Math::Vector4 uAmbientLight;
	//float cascadePlaneDistances[NR_CASCADES]; doesn't work for whatever reason
	float cascadePlaneDistances[4];
	float uShininess = 32.0f; // TODO: remove from lighting
};
__declspec(align(16))
struct InversedProj {
	Math::Matrix InverseProjection;
	Math::Vector2 ScreenDimensions;
};

DeferredLightingPass::DeferredLightingPass(RenderContext& ctx) 
{
    const D3D11_BUFFER_DESC cbPSDescs[] = {
		{
		    sizeof(CBPS),	// UINT ByteWidth;
		    D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
		    D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
		    D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
		    0,							// UINT MiscFlags;
		    0,							// UINT StructureByteStride;
		},
		{
			sizeof(InversedProj),	// UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
			D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
			0,							// UINT MiscFlags;
			0,							// UINT StructureByteStride;
		},
		{
			sizeof(Light::CBPS),	// UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
			D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
			0,							// UINT MiscFlags;
			0,							// UINT StructureByteStride;
		}
    };
    shader = std::make_shared<Shader>(L"shaders/DeferredLightingPass.hlsl", (Shader::CreationFlags)(Shader::VERTEX_SH | Shader::PIXEL_SH), ctx.device, nullptr, 0, nullptr, 0, cbPSDescs, (int)std::size(cbPSDescs));
}

void DeferredLightingPass::AddDeferredLightingPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard)
{
	using namespace Triad::Render::Api;

	const auto& gBuffer = bboard.get<GBufferPass::GBufferPassData>();
	bboard.add<DeferredLightingPassData>() = fg.addCallbackPass<DeferredLightingPassData>("DeferredLightingPass",
		[&](FrameGraph::Builder& builder, DeferredLightingPassData& data) {
			builder.setSideEffect();
			decltype(FrameGraphResources::FGTexture::Desc::texDesc) texDesc = {};
			texDesc.MipLevels = 1;
			texDesc.Format = DXGI_FORMAT_R32G32B32A32_TYPELESS;
			texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.ArraySize = 1;
			data.sceneColor = builder.create<FrameGraphResources::FGTexture>("SceneColor", {
				.texDesc = texDesc,
				.fitToViewport = true
				});

			builder.read(gBuffer.albedoSpec);
			builder.read(gBuffer.normal);
			builder.read(gBuffer.depthStencil);
			builder.read(gBuffer.lightAcc);
			data.sceneColor = builder.write(data.sceneColor);
		},
		[=, this, &ctx](const DeferredLightingPassData& data, FrameGraphPassResources& resources, void*) {
			ctx.ClearState();

			ctx->RSSetState(ctx.masterRastState);
			ctx->PSSetSamplers(0, 1, &ctx.masterSamplerState);
			ctx->RSSetViewports(1, ctx.viewport.Get11());

			// rgba color textures
			D3D11_SHADER_RESOURCE_VIEW_DESC srtDesc = {};
			srtDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			srtDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srtDesc.Texture2D.MipLevels = 1;

			// color (albedo specular)
			auto& albedoSpec = resources.get<FrameGraphResources::FGTexture>(gBuffer.albedoSpec);
			ShaderResource* albedoSpecSrt = albedoSpec.BindRead(ctx, srtDesc, 0);

			// normals
			auto& normal = resources.get<FrameGraphResources::FGTexture>(gBuffer.normal);
			ShaderResource* normalSrt = normal.BindRead(ctx, srtDesc, 1);

			// light accumulation buffer
			auto& lightAcc = resources.get<FrameGraphResources::FGTexture>(gBuffer.lightAcc);
			ShaderResource* lightAccRtv = lightAcc.BindRead(ctx, srtDesc, 3);

			// depth
			auto& depthStencil = resources.get<FrameGraphResources::FGTexture>(gBuffer.depthStencil);
			D3D11_SHADER_RESOURCE_VIEW_DESC depthViewDesc = {};
			depthViewDesc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			depthViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			depthViewDesc.Texture2D.MipLevels = 1;
			depthViewDesc.Texture2D.MostDetailedMip = 0;
			ShaderResource* depthBuf = depthStencil.BindRead(ctx, depthViewDesc, 2);

			auto& sceneColor = resources.get<FrameGraphResources::FGTexture>(data.sceneColor);
			RenderTargetDesc rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			RenderTarget* rtv = sceneColor.BindWrite(ctx, &rtvDesc, 0);

			Light::CBPS lightBuffer;
			for (const auto& dirLightSrc : LightsStorage::Instance().dirLights.GetStorage()) {
				dirLightSrc.light->UpdateBuffer(lightBuffer, dirLightSrc.transform); // TEMP untill dir light gets extracted to a separate full screen pass
				break;
			}

			CBPS cbPSLight;
			cbPSLight.inverseViewMatr = gTempGame->GetActiveCamera()->GetViewMatrix().Invert().Transpose();  // Do we need use EditorCamera instead here in editor(?)
			cbPSLight.viewMatr = gTempGame->GetActiveCamera()->GetViewMatrix().Transpose();					 // Do we need use EditorCamera instead here in editor(?)
			CameraComponent* cam = static_cast<CameraComponent*>(gTempGame->GetActiveCamera());				 // Do we need use EditorCamera instead here in editor(?)
			cbPSLight.uCameraPos = Math::Vector4(cam->GetCameraPos());

			shader->SetCBPS(ctx.context, 0, &cbPSLight);
			{
				InversedProj inversedProj;
				// Do we need use EditorCamera instead here in editor(?)
				inversedProj.InverseProjection = gTempGame->GetActiveCamera()->GetProjectionMatrix().Invert().Transpose(); // cam inversed proj
				inversedProj.ScreenDimensions = Math::Vector2{ ctx.viewport.width, ctx.viewport.height };
				shader->SetCBPS(ctx.context, 1, &inversedProj);
			}
			shader->SetCBPS(ctx.context, 2, &lightBuffer);

			ctx.TEMP_UpdateRenderTargetsNum();
			ctx->OMSetRenderTargets(ctx.activeRenderTargetNum, ctx.activeRenderTargets, ctx.activeDepthBuffuer);
			shader->Activate(ctx, shader);
			DrawFullScreenQuad(ctx);
		});
}

void DeferredLightingPass::DrawFullScreenQuad(RenderContext& ctx)
{
	ctx->IASetInputLayout(nullptr);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ctx->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	ctx->Draw(4, 0);
}
