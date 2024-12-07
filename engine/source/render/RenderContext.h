#pragma once

#include "drv/d3d11/D3D11Helpers.h"
#include "Shader.h"
#include "os/Window.h"
#include "math/Math.h"

struct RenderContext {
	void ResetFrame();
	void ClearState();

	void ResizeViewport(const Math::Viewport& viewport);

	void TEMP_UpdateRenderTargetsNum();

	ID3D11DeviceContext* operator->() { return context; }

	bool viewportResized = false;
	bool backbuffResized = false;
	Math::Viewport viewport;
	Window* window;

	std::shared_ptr<Shader> activeShader;

	// mouse picking support
	uint32_t entityIdUnderCursor = 0;

	// d3d11 part
	ID3D11Device* device;
	ID3D11DeviceContext* context;

	ID3D11RasterizerState* masterRastState;
	ID3D11SamplerState* masterSamplerState;

	// current OM state
	uint32_t activeRenderTargetNum = 0;
	static constexpr int MAX_ACTIVE_RENDER_TARGETS = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
	Triad::Render::Api::RenderTarget* activeRenderTargets[MAX_ACTIVE_RENDER_TARGETS];
	Triad::Render::Api::DepthBuffer* activeDepthBuffuer = nullptr;

	IDXGISwapChain* swapChain;
};
