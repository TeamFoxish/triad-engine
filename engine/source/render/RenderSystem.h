#pragma once

#include <memory>

class RuntimeIface;
class Renderer;

class RenderSystem {
public:
	RenderSystem() = default;
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	~RenderSystem() = default;

	bool Init(RuntimeIface* runtime);
	void Term();

	Renderer* GetRenderer() const { return rendererImpl.get(); }

	void StartFrame();
	void EndFrame();

protected:
	std::unique_ptr<Renderer> rendererImpl;
};

extern std::unique_ptr<RenderSystem> gRenderSys;
