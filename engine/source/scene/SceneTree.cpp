#include "SceneTree.h"

std::unique_ptr<SceneTree> gSceneTree;

auto SceneTree::Add(Entity&& entity) -> Handle
{
    Handle parent = entity.parent;
    Handle handle = storage.Add(std::move(entity));
    if (parent.id_ >= 0) {
		Entity& parentEntity = storage[parent];
		parentEntity.children.push_back(handle);
    }
	if (root.id_ < 0) {
		root = handle;
	}
	OnEntityAdded.Broadcast(handle);
    return handle;
}

void SceneTree::Remove(Handle handle)
{
	OnEntityRemovedBefore.Broadcast(handle);
    Entity& entity = storage[handle];
	// assert(entry.children.empty()); // too hard to support such behavior (a lot of circular references in scripts)
	if (entity.parent.id_ >= 0) {
		Entity* parent = storage.Get(entity.parent);
		if (parent) {
			const auto iter = std::find(parent->children.begin(), parent->children.end(), handle);
			assert(iter != parent->children.end());
			parent->children.erase(iter);
			entity.parent = Handle{};
		}
	}
	if (handle == root) {
		root = Handle{}; // scene root removed
	}
	storage.Remove(handle);
	OnEntityRemovedAfter.Broadcast(handle);
}

void InitSceneTree()
{
	gSceneTree = std::make_unique<SceneTree>();
}

void TermSceneTree()
{
	gSceneTree.reset();
}
