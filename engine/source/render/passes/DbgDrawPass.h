#pragma once

#ifdef EDITOR

#include "fg/FrameGraphResource.hpp"
#include "render/FrameGraphResources.h"

class FrameGraph;
class FrameGraphBlackboard;

class DbgDrawPass {
public:
	struct DbgDrawPassData {
	};

	DbgDrawPass(RenderContext& ctx);

	static void AddDbgDrawPass(RenderContext& ctx, FrameGraph& fg, FrameGraphBlackboard& bboard);

protected:
	static void DebugDraw(RenderContext& ctx);
};

#endif // EDITOR
