#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#include "RenderContext.h"
#include "RenderUtils.h"
#include "DrawComponent.h"
#include "materials/DefaultMeshMaterial.h" // TODO: TEMP E1
#include "Mesh.h" // TODO: forward declare

class Window;
class Light;
struct Shader;

class Renderer {
	friend DrawComponent::DrawComponent(Game*, Compositer*);
	friend DrawComponent::~DrawComponent();
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

	void DrawScene();
	void DrawScreenQuad();

	void SetClearColor(const float* color);

	void PopulateLightsBuffer(DefaultMeshMaterial::CBPS& buffer) const;

	const Math::Matrix& GetViewMatrix() const { return viewMatr; }
	void SetViewMatrix(const Math::Matrix& view) { viewMatr = view; }

	Mesh::PTR GetMesh(const std::string& path);
	ID3D11ShaderResourceView* GetTexture(const std::wstring& path);

	Window* GetWindow() const { return window; }
	RenderUtils* GetUtils() const { return utils.get(); }
	ID3D11Device* GetDevice() const { return context.device; }
	ID3D11DeviceContext* GetDeviceContext() const { return context.context; }

	ID3D11ShaderResourceView* GetColorPassSrt() const { return colorPassSrt; }

private:
	void AddComponent(DrawComponent* comp);
	void RemoveComponent(DrawComponent* comp);

	void AddLight(Light* light);
	void RemoveLight(Light* light);

	void TestFrameGraph();

private:
	std::unique_ptr<RenderUtils> utils;

	std::vector<DrawComponent*> components;

	std::unordered_map<std::string, Mesh::PTR> meshes;

	std::unordered_map<std::wstring, ID3D11ShaderResourceView*> textures;

	std::vector<Light*> lightSources;

	Window* window = nullptr;

	RenderContext context;

	Shader* texToBackBuffShader = nullptr; // TEMP

	ID3D11ShaderResourceView* colorPassSrt = nullptr; // TEMP

	ID3D11DepthStencilState* pDSState; // TEMP

	Math::Matrix viewMatr;

	float clearColor[4]{ 0.1f, 0.1f, 0.1f, 1.0f };
};
