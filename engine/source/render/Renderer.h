#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#include "RenderContext.h"
#include "RenderUtils.h"
#include "light/Lights.h"
#include "mesh/Mesh.h" // TODO: forward declare
#include "DeferredRenderer.h" // TODO: forward declare

class Window;
class Light;
struct Shader;

class Renderer {
	friend class Light;

public:
	Renderer() = default;
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	~Renderer() = default;

	bool Initialize(Window* window);
	void Shutdown();

	void Draw();
	void EndFrame();

	void DrawScreenQuad();

	void ResizeBackBuff();

	void SetClearColor(const float* color);

	const Math::Matrix& GetViewProjMatrix() const { return viewMatr; }
	void SetViewMatrix(const Math::Matrix& view) { viewMatr = view; }

	Mesh::PTR GetMesh(const std::string& path);

	Window* GetWindow() const { return window; }
	RenderUtils* GetUtils() const { return utils.get(); }
	ID3D11Device* GetDevice() const { return context.device; }
	ID3D11DeviceContext* GetDeviceContext() const { return context.context; }
	RenderContext& GetContext() { return context; }

	ID3D11ShaderResourceView* GetColorPassSrt() const { return colorPassSrt; }

	uint32_t GetEntityIdUnderCursor() const { return context.entityIdUnderCursor; }

private:
	void AddLight(Light* light);
	void RemoveLight(Light* light);

	void TestFrameGraph();

private:
	std::unique_ptr<RenderUtils> utils;

	std::unordered_map<std::string, Mesh::PTR> meshes;

	std::vector<Light*> lightSources;

	Window* window = nullptr;

	RenderContext context;

	std::shared_ptr<Shader> texToBackBuffShader; // TEMP

	ID3D11ShaderResourceView* colorPassSrt = nullptr; // TEMP

	ID3D11DepthStencilState* pDSState; // TEMP

	Triad::Render::Api::RenderTarget* mainRtv = nullptr; // TEMP

	Math::Matrix viewMatr;

	std::unique_ptr<DeferredRenderer> deferredRenderer;

	float clearColor[4]{ 0.1f, 0.1f, 0.1f, 1.0f };
};
