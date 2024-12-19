#pragma once

#include "shared/TransformStorage.h"
#include "Camera.h"

class CameraStorage {
	friend class CameraManager;

public:
	struct CameraEntry;

	using Storage = HandleStorage<CameraEntry>;
	using Handle = Storage::Handle;

	struct CameraEntry {
		Camera camera;
		Camera::Params params;
		TransformStorage::Handle transform;
		bool updateProjection = false;
	};

	CameraStorage() = default;
	CameraStorage(const CameraStorage&) = delete;
	CameraStorage(TransformStorage&&) = delete;
	~CameraStorage() = default;

	CameraEntry& Get(Handle handle) { return storage[handle]; }

	Handle Add(const Camera::Params& params, TransformStorage::Handle transform);
	void Remove(Handle handle) { storage.Remove(handle); }

private:
	Storage storage;
};
