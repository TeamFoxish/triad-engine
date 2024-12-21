#include "render/RenderContext.h"
#include "LightVolumesPass.h"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"

#include "render/RenderSystem.h"
#include "render/Renderer.h"
#include "render/GeometryData.h"
#include "render/mesh/MeshRenderer.h"
#include "render/light/LightsStorage.h"
#include "shared/SharedStorage.h"
#include "GBufferPass.h"

// TEMP
#include "runtime/EngineRuntime.h"
#include "game/Game.h"
#include "components/CameraComponent.h"

__declspec(align(16))
struct CBPSVolumes {
	Math::Matrix viewMatr;
	float uShininess = 32.0f; // TODO: remove?
};

__declspec(align(16))
struct InversedProj {
	Math::Matrix InverseProjection;
	Math::Vector2 ScreenDimensions;
};

LightVolumesPass::LightVolumesPass(RenderContext& ctx)
{
	{
		CD3D11_RASTERIZER_DESC rastDesc = {};
		rastDesc.CullMode = D3D11_CULL_FRONT;
		rastDesc.FillMode = D3D11_FILL_SOLID;

		ctx.device->CreateRasterizerState(&rastDesc, &rastState1);

		D3D11_DEPTH_STENCIL_DESC dsDesc;

		// Depth test parameters
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;

		// Stencil test parameters
		dsDesc.StencilEnable = true;
		dsDesc.StencilReadMask = 0xFF;
		dsDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_DECR_SAT;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_DECR_SAT;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create depth stencil state
		ctx.device->CreateDepthStencilState(&dsDesc, &dsState1);
	}
	{
		CD3D11_RASTERIZER_DESC rastDesc = {};
		rastDesc.CullMode = D3D11_CULL_BACK;
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.DepthClipEnable = FALSE;

		ctx.device->CreateRasterizerState(&rastDesc, &rastState2);

		D3D11_DEPTH_STENCIL_DESC dsDesc;

		// Depth test parameters
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;

		// Stencil test parameters
		dsDesc.StencilEnable = true;
		dsDesc.StencilReadMask = 0xFF;
		dsDesc.StencilWriteMask = 0x00;

		// Stencil operations if pixel is front-facing
		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

		// Stencil operations if pixel is back-facing
		dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

		// Create depth stencil state
		ctx.device->CreateDepthStencilState(&dsDesc, &dsState2);
	}
	{
		D3D11_RENDER_TARGET_BLEND_DESC rtbd;
		ZeroMemory(&rtbd, sizeof(rtbd));
		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND_ONE;
		rtbd.DestBlend = D3D11_BLEND_ONE;
		rtbd.BlendOp = D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.RenderTarget[0] = rtbd;
		ctx.device->CreateBlendState(&blendDesc, &blendState);
	}
	{
		const D3D11_INPUT_ELEMENT_DESC inputElements[] = {
			D3D11_INPUT_ELEMENT_DESC {
				"POSITION",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				0,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			D3D11_INPUT_ELEMENT_DESC {
				"NORMAL",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			D3D11_INPUT_ELEMENT_DESC {
				"TEXCOORD",
				0,
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
		};
		const D3D11_BUFFER_DESC cbVSDescs[] = {
			{
				sizeof(MeshRenderer::CBVS), // UINT ByteWidth;
				D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
				D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
				D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
				0,							// UINT MiscFlags;
				0,							// UINT StructureByteStride;
			}
		};

		pureGeomShader = std::make_shared<Shader>(L"shaders/PureGeometryPass.hlsl", Shader::VERTEX_SH, ctx.device, inputElements, (int)std::size(inputElements), cbVSDescs, 1, nullptr, 0);
	}
	{
		const D3D11_INPUT_ELEMENT_DESC inputElements[] = {
			D3D11_INPUT_ELEMENT_DESC {
				"POSITION",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				0,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			D3D11_INPUT_ELEMENT_DESC {
				"NORMAL",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			D3D11_INPUT_ELEMENT_DESC {
				"TEXCOORD",
				0,
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
		};
		const D3D11_BUFFER_DESC cbVSDescs[] = {
			{
				sizeof(MeshRenderer::CBVS), // UINT ByteWidth;
				D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
				D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
				D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
				0,							// UINT MiscFlags;
				0,							// UINT StructureByteStride;
			}
		};
		const D3D11_BUFFER_DESC cbPSDescs[] = {
			{
				sizeof(CBPSVolumes),            // UINT ByteWidth;
				D3D11_USAGE_DYNAMIC,		    // D3D11_USAGE Usage;
				D3D11_BIND_CONSTANT_BUFFER,     // UINT BindFlags;
				D3D11_CPU_ACCESS_WRITE,		    // UINT CPUAccessFlags;
				0,							    // UINT MiscFlags;
				0,							    // UINT StructureByteStride;
			},
			{
				sizeof(InversedProj),            // UINT ByteWidth;
				D3D11_USAGE_DYNAMIC,		    // D3D11_USAGE Usage;
				D3D11_BIND_CONSTANT_BUFFER,     // UINT BindFlags;
				D3D11_CPU_ACCESS_WRITE,		    // UINT CPUAccessFlags;
				0,							    // UINT MiscFlags;
				0,							    // UINT StructureByteStride;
			},
			{
				sizeof(Light::CBPS),            // UINT ByteWidth;
				D3D11_USAGE_DYNAMIC,		    // D3D11_USAGE Usage;
				D3D11_BIND_CONSTANT_BUFFER,     // UINT BindFlags;
				D3D11_CPU_ACCESS_WRITE,		    // UINT CPUAccessFlags;
				0,							    // UINT MiscFlags;
				0,							    // UINT StructureByteStride;
			}
		};

		lightVolumesShader = std::make_shared<Shader>(L"shaders/LightVolumesPass.hlsl", (Shader::CreationFlags)(Shader::VERTEX_SH | Shader::PIXEL_SH), ctx.device, inputElements, (int)std::size(inputElements), cbVSDescs, 1, cbPSDescs, (int)std::size(cbPSDescs));
	}
}

void LightVolumesPass::AddLightVolumesPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard)
{
	using namespace Triad::Render::Api;

	if (!pureGeomShader || !lightVolumesShader) {
		return;
	}

	auto& gBuffer = bboard.get<GBufferPass::GBufferPassData>();
	bboard.add<LightVolumesPassData>() = fg.addCallbackPass<LightVolumesPassData>("LightingVolumesPass",
		[&](FrameGraph::Builder& builder, LightVolumesPassData& data) {
			// create a copy for depth buffer
			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.Format = DXGI_FORMAT_R32G8X24_TYPELESS;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			data.depthStencilCopy = builder.create<FrameGraphResources::FGTexture>("SceneDepthCopy", {
				.texDesc = texDesc,
				.fitToViewport = true
			});

			gBuffer.albedoSpec = builder.read(gBuffer.albedoSpec);
			gBuffer.normal = builder.read(gBuffer.normal);
			gBuffer.lightAcc = builder.write(gBuffer.lightAcc);
			gBuffer.depthStencil = builder.write(gBuffer.depthStencil);
			data.depthStencilCopy = builder.write(data.depthStencilCopy);
			builder.setSideEffect();
		},
		[=, this, &ctx](const LightVolumesPassData& data, FrameGraphPassResources& resources, void*) {
			ctx.ClearState();

			// setup pipiline
			ctx->PSSetSamplers(0, 1, &ctx.masterSamplerState);
			ctx->RSSetViewports(1, ctx.viewport.Get11());

			// depth
			auto& depthStencil = resources.get<FrameGraphResources::FGTexture>(gBuffer.depthStencil);
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			DepthBuffer* depthBuf = depthStencil.BindWrite(ctx, dsvDesc);

			// rgba color textures
			RenderTargetDesc rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
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
			RenderTarget* lightAccRtv = lightAcc.BindWrite(ctx, &rtvDesc, 0);

			{
				const CameraStorage::CameraEntry& activeCam = gRenderSys->cameraManager.GetActiveCamera();
				Renderer* renderer = gRenderSys->GetRenderer();
				auto sphereGeom = renderer->GetUtils()->GetSphereGeom(renderer);

				CBPSVolumes cbPSVolumes;
				cbPSVolumes.viewMatr = activeCam.camera.GetViewMatrix().Transpose(); // Do we need use EditorCamera instead here in editor(?)

				Light::CBPS lightBuffer;
				for (const auto& dirLightSrc : LightsStorage::Instance().dirLights.GetStorage()) {
					dirLightSrc.light->UpdateBuffer(lightBuffer, dirLightSrc.transform); // TEMP untill dir light gets extracted to a separate full screen pass
					break;
				}
				{
					// TODO: cache this buffer value since it doesn't change between passes
					InversedProj inversedProj;
					// Do we need use EditorCamera instead here in editor(?)
					inversedProj.InverseProjection = activeCam.camera.GetProjectionMatrix().Invert().Transpose(); // cam inversed proj
					inversedProj.ScreenDimensions = Math::Vector2{ctx.viewport.width, ctx.viewport.height};
					lightVolumesShader->SetCBPS(ctx.context, 1, &inversedProj);
				}

				const auto& lightsStorage = LightsStorage::Instance().pointLights.GetStorage();
				for (const auto& lightSrc : lightsStorage) {
					// fill light constant buffer
					lightSrc.light->UpdateBuffer(lightBuffer, lightSrc.transform);

					// STENCIL PASS
					ctx->ClearDepthStencilView(depthBuf, D3D11_CLEAR_STENCIL, 1.0f, 1);
					ctx->OMSetDepthStencilState(dsState1, 1);
					ctx->RSSetState(rastState1);
					pureGeomShader->Activate(ctx, pureGeomShader);

					sphereGeom->Activate(ctx.context);
					
					// fill vertex constant buffer
					MeshRenderer::CBVS cbVS;
					const Math::Transform& lightTrs = SharedStorage::Instance().transforms.AccessRead(lightSrc.transform);
					cbVS.worldTransform = (Math::Matrix::CreateScale(lightSrc.light->GetRadius()) * Math::Matrix::CreateTranslation(lightTrs.GetPosition())).Transpose();
					cbVS.viewProj = gRenderSys->cameraManager.GetViewProjTransposed(); // already transposed
					pureGeomShader->SetCBVS(ctx.context, 0, &cbVS);
					
					ctx->OMSetRenderTargets(0, nullptr, depthBuf);
					ctx->DrawIndexed(sphereGeom->idxNum, 0, 0);

					// COLOR PASS
					RenderTarget* rtv = lightAcc.BindWrite(ctx, &rtvDesc, 0);
					ctx.TEMP_UpdateRenderTargetsNum();
					ctx->OMSetRenderTargets(1, &lightAccRtv, depthBuf);
					ctx->OMSetDepthStencilState(dsState2, 1);
					ctx->RSSetState(rastState2);
					float blendFactor[] = {1.0f, 1.0f, 1.0f, 1.0f};
					ctx->OMSetBlendState(blendState, blendFactor, 0xffffffff);

					lightVolumesShader->Activate(ctx, lightVolumesShader);
					lightVolumesShader->SetCBVS(ctx.context, 0, &cbVS);
					lightVolumesShader->SetCBPS(ctx.context, 0, &cbPSVolumes);
					lightVolumesShader->SetCBPS(ctx.context, 2, &lightBuffer);

					ctx->PSSetShaderResources(0, 1, &albedoSpecSrt);
					ctx->PSSetShaderResources(1, 1, &normalSrt);

					// copy depth buffer
					auto& depthBufCopy = resources.get<FrameGraphResources::FGTexture>(data.depthStencilCopy);
					ctx->CopyResource(depthBufCopy.tex, depthStencil.tex);

					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					ZeroMemory(&srvDesc, sizeof(srvDesc));
					srvDesc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MipLevels = 1;
					srvDesc.Texture2D.MostDetailedMip = 0;
					depthBufCopy.BindRead(ctx, srvDesc, 2);

					sphereGeom->Activate(ctx.context);
					ctx->DrawIndexed(sphereGeom->idxNum, 0, 0);
				}
			}
		}
	);
}
