#pragma once

#include "fg/FrameGraphResource.hpp"
#include "render/FrameGraphResources.h"
#include "render/Shader.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"

class FrameGraph;
class FrameGraphBlackboard;

class UIPass {
public:
	struct UIPassData {
	};

	UIPass(RenderContext& ctx);

	void AddUIPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard);

protected:
	void DrawUI(RenderContext& ctx);

protected:

	std::unique_ptr<DirectX::SpriteFont> _font = nullptr;
	std::unique_ptr<DirectX::SpriteBatch> _batch = nullptr;

	float clearColor[4] {0.1f, 0.1f, 0.1f, 1.0f};
};
