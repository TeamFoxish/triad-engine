#pragma once

#include <memory>

#include "camera/CameraManager.h"

#include "misc/Delegates.h"

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

	struct RenderContext& GetContext() const;
	Renderer* GetRenderer() const { return rendererImpl.get(); }

	void StartFrame();
	void EndFrame();

	uint32_t GetEntityIdUnderCursor() const;

public:
	CameraManager cameraManager;

protected:
	std::unique_ptr<Renderer> rendererImpl;

	DelegateHandle viewportResizedHandle;
};

extern std::unique_ptr<RenderSystem> gRenderSys;
