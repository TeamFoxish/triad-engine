#pragma once

#include "fg/FrameGraphResource.hpp"
#include "render/FrameGraphResources.h"
#include "render/Shader.h"

class FrameGraph;
class FrameGraphBlackboard;

class GBufferPass {
public:
	struct GBufferPassData {
		FrameGraphResource albedoSpec;
		FrameGraphResource normal;
		FrameGraphResource depthStencil;
		FrameGraphResource lightAcc;
		FrameGraphResource entityIds;
		FrameGraphResource entityIdsCopy;
	};

	GBufferPass(RenderContext& ctx);

	void AddGeometryPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard);

protected:
	void DrawSceneGeometry(RenderContext& ctx);

	void DrawSceneLighting(RenderContext& ctx);

	void QueryEntityUnderCursor(RenderContext& ctx, FrameGraphResources::FGTexture& idsTexture, FrameGraphResources::FGTexture& idsCopy);

protected:
	struct ID3D11DepthStencilState* dsState = nullptr;

	std::shared_ptr<Shader> shader;

	float clearColor[4] {0.1f, 0.1f, 0.1f, 1.0f};
};
