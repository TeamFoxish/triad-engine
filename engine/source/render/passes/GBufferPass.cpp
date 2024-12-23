#include "render/RenderContext.h" // place at the very top, so d3d11 is included first and Math::Viewport::Get11() is defined later

#include "GBufferPass.h"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"

#include "render/Renderable.h"
#include "render/mesh/MeshRenderer.h"

// mouse picking support
#include "input/InputDevice.h"
#include "editor/ui_debug/UIDebug.h"

GBufferPass::GBufferPass(RenderContext& ctx)
{
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Create depth stencil state
	ctx.device->CreateDepthStencilState(&dsDesc, &dsState);

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
			sizeof(MeshRenderer::CBPSGeom), // UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,		    // D3D11_USAGE Usage;
			D3D11_BIND_CONSTANT_BUFFER,     // UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,		    // UINT CPUAccessFlags;
			0,							    // UINT MiscFlags;
			0,							    // UINT StructureByteStride;
		}
	};

	shader = std::make_shared<Shader>(L"shaders/GBufferPass.hlsl", (Shader::CreationFlags)(Shader::VERTEX_SH | Shader::PIXEL_SH), ctx.device, inputElements, (int)std::size(inputElements), cbVSDescs, 1, cbPSDescs, 1);
}

void GBufferPass::AddGeometryPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard)
{
	using namespace Triad::Render::Api;

	if (!shader) {
		return;
	}

    bboard.add<GBufferPassData>() = fg.addCallbackPass<GBufferPassData>("GBufferPass",
        [&](FrameGraph::Builder& builder, GBufferPassData& data) {
			{
				builder.setSideEffect();
				decltype(FrameGraphResources::FGTexture::Desc::texDesc) texDesc = {};
				texDesc.MipLevels = 1;
				texDesc.Format = DXGI_FORMAT_R32G32B32A32_TYPELESS;
				texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.ArraySize = 1;
				data.albedoSpec = builder.create<FrameGraphResources::FGTexture>("SceneColor", { 
				    .texDesc = texDesc,
				    .fitToViewport = true
				});
				data.normal = builder.create<FrameGraphResources::FGTexture>("SceneNormals", { 
				    .texDesc = texDesc,
				    .fitToViewport = true
				});
				data.lightAcc = builder.create<FrameGraphResources::FGTexture>("LightAcc", { 
				    .texDesc = texDesc,
				    .fitToViewport = true
				});
			}
			{
				decltype(FrameGraphResources::FGTexture::Desc::texDesc) texDesc = {};
				texDesc.MipLevels = 1;
				texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.ArraySize = 1;
				data.entityIds = builder.create<FrameGraphResources::FGTexture>("EntityIds", {
				    .texDesc = texDesc,
				    .fitToViewport = true
				});
			}
			{   // a single pixel entity id texture
				decltype(FrameGraphResources::FGTexture::Desc::texDesc) texDesc = {};
				texDesc.MipLevels = 1;
				texDesc.Width = 1;
				texDesc.Height = 1;
				texDesc.Usage = D3D11_USAGE_STAGING;
				texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				texDesc.Format = DXGI_FORMAT_R32_SINT;
				//texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.ArraySize = 1;
				data.entityIdsCopy = builder.create<FrameGraphResources::FGTexture>("EntityIdsCopy", {
				    .texDesc = texDesc,
				    .fitToViewport = false
				});
			}
			{
				decltype(FrameGraphResources::FGTexture::Desc::texDesc) depthTexDesc = {};
				ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
				depthTexDesc.MipLevels = 1;
				depthTexDesc.ArraySize = 1;
				depthTexDesc.SampleDesc.Count = 1;
				depthTexDesc.Format = DXGI_FORMAT_R32G8X24_TYPELESS;
				depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				depthTexDesc.SampleDesc.Count = 1;
				depthTexDesc.SampleDesc.Quality = 0;
				data.depthStencil = builder.create<FrameGraphResources::FGTexture>("SceneDepth", { 
				    .texDesc = depthTexDesc,
				    .fitToViewport = true
				});
			}
            data.albedoSpec = builder.write(data.albedoSpec);
            data.normal = builder.write(data.normal);
			data.depthStencil = builder.write(data.depthStencil);
			data.lightAcc = builder.write(data.lightAcc);
			data.entityIds = builder.write(data.entityIds);
			data.entityIdsCopy = builder.write(data.entityIdsCopy);
        },
        [=, this, &ctx](const GBufferPassData& data, FrameGraphPassResources& resources, void*) {
			ctx.ClearState();

			// setup pipiline
			ctx->RSSetState(ctx.masterRastState);
			ctx->PSSetSamplers(0, 1, &ctx.masterSamplerState);
			ctx->RSSetViewports(1, ctx.viewport.Get11());
			ctx->OMSetDepthStencilState(dsState, 1);

			{	// rgba color textures
				RenderTargetDesc rtvDesc = {};
				rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				{   // color (albedo specular)
					auto& albedoSpec = resources.get<FrameGraphResources::FGTexture>(data.albedoSpec);
					RenderTarget* rtv = albedoSpec.BindWrite(ctx, &rtvDesc, 0);
					ctx->ClearRenderTargetView(rtv, clearColor);
				}
				{   // normals
					auto& normal = resources.get<FrameGraphResources::FGTexture>(data.normal);
					RenderTarget* rtv = normal.BindWrite(ctx, &rtvDesc, 1);
					ctx->ClearRenderTargetView(rtv, clearColor);
				}
				{   // light accumulation buffer
					auto& lightAcc = resources.get<FrameGraphResources::FGTexture>(data.lightAcc);
					RenderTarget* rtv = lightAcc.BindWrite(ctx, &rtvDesc, 2);
					ctx->ClearRenderTargetView(rtv, clearColor);
				}
			}

			// depth
			auto& depthStencil = resources.get<FrameGraphResources::FGTexture>(data.depthStencil);
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			DepthBuffer* depthBuf = depthStencil.BindWrite(ctx, dsvDesc);
			ctx->ClearDepthStencilView(depthBuf, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			// enitity ids
			auto& idsTexture = resources.get<FrameGraphResources::FGTexture>(data.entityIds);
			RenderTargetDesc rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R32_SINT;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			RenderTarget* rtv = idsTexture.BindWrite(ctx, &rtvDesc, 3);
			ctx->ClearRenderTargetView(rtv, clearColor);

			ctx.TEMP_UpdateRenderTargetsNum();
			ctx->OMSetRenderTargets(ctx.activeRenderTargetNum, ctx.activeRenderTargets, ctx.activeDepthBuffuer);

			shader->Activate(ctx, shader);
			
			// draw scene to gbuffer
			DrawSceneGeometry(ctx);

			// mouse picking support
			ctx->OMSetRenderTargets(0, nullptr, nullptr);
			auto& idsCopy = resources.get<FrameGraphResources::FGTexture>(data.entityIdsCopy);
			QueryEntityUnderCursor(ctx, idsTexture, idsCopy);
        }
    );
}

void GBufferPass::DrawSceneGeometry(RenderContext& ctx)
{
	for (const Renderable& renderObj : RenderableStorage::Instance().GetStorage()) {
		MeshRenderer::DrawGeometryOnly(ctx, renderObj);
	}
}

void GBufferPass::QueryEntityUnderCursor(RenderContext& ctx, FrameGraphResources::FGTexture& idsTexture, FrameGraphResources::FGTexture& idsCopy)
{
	// copy entities id texture region and cache entity id under the cursor
	// TODO: use imgui cursor pos instead?
	Math::Vector2 mousePos = globalInputDevice->MousePosition;
#ifdef EDITOR // TODO: should be aquired as input->GetMousePos()
	mousePos.x -= UIDebug::GetViewportX();
	mousePos.y -= UIDebug::GetViewportY();
	mousePos.y -= 10; // TEMP no idea why cursor losing about 10 units by Y axis when fetching texture 0_o
#endif
	ctx.entityIdUnderCursor = -1;
	if (mousePos.x < 0.0f || mousePos.x > ctx.viewport.width || mousePos.y < 0.0f || mousePos.y > ctx.viewport.height) {
		return;
	}
	// get a single pixel texture with int32 content
	D3D11_BOX srcRegion;
	srcRegion.left = (uint32_t)mousePos.x;
	srcRegion.right = srcRegion.left + 1;
	srcRegion.top = (uint32_t)mousePos.y;
	srcRegion.bottom = srcRegion.top + 1;
	srcRegion.front = 0;
	srcRegion.back = 1;
	// copy a single pixel region to idsCopy texture
	ctx->CopySubresourceRegion(idsCopy.tex, 0, 0, 0, 0, idsTexture.tex, 0, &srcRegion);
	// read resulted id
	D3D11_MAPPED_SUBRESOURCE destRes = {};
	ctx->Map(idsCopy.tex, 0, D3D11_MAP_READ, 0, &destRes);
	ctx.entityIdUnderCursor = static_cast<int32_t*>(destRes.pData)[0];
	ctx->Unmap(idsCopy.tex, 0);
}
