#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "RenderUtils.h"
#include "DrawComponent.h"
#include "materials/DefaultMeshMaterial.h" // TODO: TEMP E1
#include "Mesh.h" // TODO: forward declare

class Window;
class Light;
struct Shader;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RasterizerState;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11DepthStencilView;
struct ID3D11DepthStencilState;

class Renderer {
	friend DrawComponent::DrawComponent(Game*, Compositer*);
	friend DrawComponent::~DrawComponent();
	friend class Light;

public:
	bool Initialize(Window* window);
	void Shutdown();
	//void UnloadData();

	void Draw();
	void EndFrame();

	void SetClearColor(const float* color);

	void PopulateLightsBuffer(DefaultMeshMaterial::CBPS& buffer) const;

	const Math::Matrix& GetViewMatrix() const { return viewMatr; }
	void SetViewMatrix(const Math::Matrix& view) { viewMatr = view; }

	Mesh::PTR GetMesh(const std::string& path);
	ID3D11ShaderResourceView* GetTexture(const std::wstring& path);

	Window* GetWindow() const { return window; }
	RenderUtils* GetUtils() const { return utils.get(); }
	ID3D11Device* GetDevice() const { return device; }
	ID3D11DeviceContext* GetDeviceContext() const { return context; }
	IDXGISwapChain* GetSwapChain() const { return swapChain; }

private:
	void AddComponent(DrawComponent* comp);
	void RemoveComponent(DrawComponent* comp);

	void AddLight(Light* light);
	void RemoveLight(Light* light);

private:
	std::unique_ptr<RenderUtils> utils;

	std::vector<DrawComponent*> components;

	std::unordered_map<std::string, Mesh::PTR> meshes;

	std::unordered_map<std::wstring, ID3D11ShaderResourceView*> textures;

	std::vector<Light*> lightSources;

	Window* window = nullptr;

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	IDXGISwapChain* swapChain = nullptr;
	ID3D11RasterizerState* rastState = nullptr;
	ID3D11ShaderResourceView* colorPassSrt = nullptr;
	ID3D11RenderTargetView* colorPassRtv = nullptr;
	ID3D11RenderTargetView* rtv;
	ID3D11SamplerState* samplerState;
	ID3D11DepthStencilView* depthBuffer;
	ID3D11DepthStencilState* pDSState;

	Shader* texToBackBuffShader = nullptr;

	Math::Matrix viewMatr;

	float clearColor[4]{ 0.1f, 0.1f, 0.1f, 1.0f };
};
