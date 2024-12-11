#pragma once

#include "mesh/Mesh.h"
#include "material/Material.h"
#include "shared/TransformStorage.h"

struct Renderable {
	struct DrawParams {
		bool isVisible = true;
		bool castShadows = true;
	};

	Renderable(uint32_t entityId, TransformStorage::Handle transform);

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
	TransformStorage::Handle transform;
	DrawParams params;
	uint32_t entityId;
};

class RenderableStorage {
	friend Renderable;
	friend bool InitRenderableStorage();
	friend void TermRenderableStorage();

public:
	using Storage = HandleStorage<Renderable>;
	using Handle = Storage::Handle;

	RenderableStorage() = default;
	RenderableStorage(const RenderableStorage&) = delete;
	RenderableStorage(RenderableStorage&&) = delete;
	~RenderableStorage() = default;

	Renderable& Get(Handle handle) { return storage[handle]; }
	const Renderable& Get(Handle handle) const { return storage[handle]; }
	Handle Add(uint32_t entityId, TransformStorage::Handle transform) { return storage.Add(Renderable{entityId, transform}); }
	void Remove(Handle handle) { storage.Remove(handle); }

	const Storage& GetStorage() const { return storage; }

	static RenderableStorage& Instance() { return *instance; }

private:
	Storage storage;

	static inline std::unique_ptr<RenderableStorage> instance;
};

bool InitRenderableStorage();
void TermRenderableStorage();
