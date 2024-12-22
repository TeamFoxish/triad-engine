#pragma once

#include "misc/Handles.h"
#include "math/Transform.h"

class TransformStorage {
public:
	struct TransformEntry;

	using Storage = HandleStorage<TransformEntry>;
	using Handle = Storage::Handle;

	struct TransformEntry {
		Math::Transform transform;
		Handle parent;
		std::vector<Handle> children;
		bool isDirty = false;
	};

	TransformStorage() = default;
	TransformStorage(const TransformStorage&) = delete;
	TransformStorage(TransformStorage&&) = delete;
	~TransformStorage() = default; // TOOD: remove all transforms manually?

	bool IsValidHandle(Handle handle) { return storage.Get(handle) != nullptr; }

	const Math::Transform& AccessRead(Handle handle);
	Math::Transform& AccessWrite(Handle handle);

	Handle Add() { return storage.Add(TransformEntry{}); }
	Handle Add(Handle parent);
	void Remove(Handle handle);

	void Attach(Handle parent, Handle child);

	// update world matricies
	void Update();

private:
	void UpdateUpward(TransformEntry& entry);
	void UpdateSince(TransformEntry& root); // downward
	
private:
	Storage storage;
};
