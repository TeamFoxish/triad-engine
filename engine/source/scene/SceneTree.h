#pragma once

#include "misc/Handles.h"
#include "misc/Delegates.h"
#include "shared/TransformStorage.h"
#include "scripts/ScriptObject.h"

#include <memory>

class SceneTree {
public:
	struct Entity;
	using Storage = HandleStorage<Entity>;
	using Handle = Storage::Handle;

	struct Entity {
		ScriptObject obj;
		Handle parent;
		TransformStorage::Handle transform;
		std::vector<Handle> children;
		std::string name;
		bool isComposite = false;
#ifdef EDITOR
		bool isSelected = false;
#endif
	};

	SceneTree() = default;
	SceneTree(const SceneTree&) = delete;
	SceneTree(SceneTree&&) = delete;
	~SceneTree() = default;

	Entity& Get(Handle handle) { return storage[handle]; }
	const Entity& Get(Handle handle) const { return storage[handle]; }

	Handle Add(Entity&& entity);
	void Remove(Handle handle);

	Handle GetRoot() const { return root; }

public:
	MulticastDelegate<Handle> OnEntityAdded;
	MulticastDelegate<Handle> OnEntityRemovedBefore;
	MulticastDelegate<Handle> OnEntityRemovedAfter;

private:
	Storage storage;
	Handle root;
};

// TEMP
extern std::unique_ptr<SceneTree> gSceneTree;

void InitSceneTree();
void TermSceneTree();
