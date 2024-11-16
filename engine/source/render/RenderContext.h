#pragma once

#include "drv/d3d11/D3D11Helpers.h"
#include "os/Window.h"
#include "math/Math.h"

struct RenderContext {
	void ResizeViewport(const Math::Viewport& viewport);

	ID3D11DeviceContext* operator->() { return context; }

	bool viewportResized = false;
	Math::Viewport viewport;
	Window* window;

	// d3d11 part
	ID3D11Device* device;
	ID3D11DeviceContext* context;

	ID3D11RasterizerState* masterRastState;
	ID3D11SamplerState* masterSamplerState;

	// current OM state
	uint32_t activeRenderTargetNum = 1;
	Triad::Render::Api::RenderTarget* activeRenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	Triad::Render::Api::DepthBuffer* activeDepthBuffuer = nullptr;

	IDXGISwapChain* swapChain;
};
