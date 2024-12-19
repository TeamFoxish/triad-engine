#pragma once

#include "camera/CameraStorage.h"

#include <memory>

struct RenderStorage {
	friend bool InitRenderStorage();
	friend void TermRenderStorage();

	RenderStorage() = default;
	RenderStorage(const RenderStorage&) = delete;
	RenderStorage(RenderStorage&&) = delete;
	~RenderStorage() = default;

	static RenderStorage& Instance() { return *instance; }

	CameraStorage cameras;

private:
	static inline std::unique_ptr<RenderStorage> instance;
};

bool InitRenderStorage();
void TermRenderStorage();
