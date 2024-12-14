#include "TransformStorage.h"

const Math::Transform& TransformStorage::AccessRead(Handle handle)
{
	TransformEntry& entry = storage[handle];
	if (entry.isDirty) {
		UpdateUpward(entry); // TODO: do not update transforms until any matrix is requested
	}
	return entry.transform;
}

Math::Transform& TransformStorage::AccessWrite(Handle handle)
{
	TransformEntry& entry = storage[handle];
	if (entry.isDirty) {
		return entry.transform;
	}
	entry.isDirty = true;
	std::vector<Handle> pending = entry.children;
	while (!pending.empty()) {
		Handle h = pending.back();
		pending.pop_back();
		TransformEntry& transform = storage[h];
		transform.isDirty = true;
		if (!transform.children.empty()) {
			pending.insert(pending.end(), transform.children.begin(), transform.children.end());
		}
	}
	return entry.transform;
}

auto TransformStorage::Add(Handle parent) -> Handle
{
	const Handle handle = Add();
	TransformEntry& entry = storage[handle];
	entry.transform = Math::Transform(Math::Matrix::Identity, Math::Matrix::Identity);
	entry.parent = parent;
	TransformEntry& parentEntry = storage[parent];
	entry.isDirty = parentEntry.isDirty;
	parentEntry.children.push_back(handle);
	return handle;
}

void TransformStorage::Remove(Handle handle)
{
	// transforms should be removed in upward order (from leafs to root)
	TransformEntry& entry = storage[handle];
	// assert(entry.children.empty()); // too hard to support such behavior (a lot of circular references in scripts)
	if (entry.parent.id_ >= 0) {
		TransformEntry* parent = storage.Get(entry.parent);
		if (parent) {
			const auto iter = std::find(parent->children.begin(), parent->children.end(), handle);
			assert(iter != parent->children.end());
			parent->children.erase(iter);
			entry.parent = Handle{};
		}
	}
	storage.Remove(handle);
}

void TransformStorage::Attach(Handle parent, Handle child)
{
	TransformEntry& parentEntry = storage[parent];
	TransformEntry& childEntry = storage[child];
	if (childEntry.parent.id_ >= 0) {
		TransformEntry& oldParent = storage[childEntry.parent];
		const auto iter = std::find(oldParent.children.begin(), oldParent.children.end(), child);
		assert(iter != oldParent.children.end());
		oldParent.children.erase(iter);
	}
	parentEntry.children.push_back(child);
	childEntry.parent = parent;
	childEntry.isDirty = parentEntry.isDirty;
	const Math::Matrix& parentMatr = parentEntry.transform.GetMatrix();
	const Math::Matrix& childMatr = childEntry.transform.GetMatrix();
	const Math::Matrix newLocal = childMatr * parentMatr.Invert();
	childEntry.transform = Math::Transform(parentEntry.transform.GetMatrix(), newLocal);
}

void TransformStorage::Update()
{
	for (TransformEntry& entry : storage) {
		if (!entry.isDirty || (entry.parent.id_ < 0 && entry.children.empty())) {
			entry.isDirty = false;
			continue;
		}
		UpdateUpward(entry);
	}
}

void TransformStorage::UpdateUpward(TransformEntry& entry)
{
	assert(entry.isDirty);
	// find the oldest parent in a dirty tree and update all its descendants
	TransformEntry* parent = &entry;
	while(parent && parent->parent.id_ >= 0) {
		TransformEntry* temp = storage.Get(parent->parent);
		assert(temp);
		if (!temp->isDirty) {
			break;
		}
		parent = temp;
	}
	UpdateSince(*parent);
}

void TransformStorage::UpdateSince(TransformEntry& root)
{
	assert(root.isDirty);
	root.isDirty = false; // root is already updated
	/*if (root.parent.id_ < 0) {
		root.transform = Math::Transform(root.transform.GetLocalMatrix());
	}*/
	std::vector<Handle> pending;
	pending.insert(pending.end(), root.children.begin(), root.children.end());
	while (!pending.empty()) {
		Handle handle = pending.back();
		pending.pop_back();
		TransformEntry& entry = storage[handle];
		assert(entry.isDirty);
		entry.isDirty = false;
		const TransformEntry& parent = storage[entry.parent];
		entry.transform = Math::Transform(parent.transform.GetMatrix(), entry.transform.GetLocalMatrix());
		pending.insert(pending.end(), entry.children.begin(), entry.children.end());
	}
}
