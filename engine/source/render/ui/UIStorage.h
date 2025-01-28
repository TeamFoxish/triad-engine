#pragma once 

#include "misc/Handles.h"
#include "UIElement.h"

#include <memory>

class UIStorage {
	friend bool InitUIStorage();
	friend void TermUIStorage();

public:
	using Storage = HandleStorage<UIElement>;
	using Handle = Storage::Handle;

	UIStorage() = default;
	UIStorage(const UIStorage&) = delete;
	UIStorage(UIStorage&&) = delete;
	~UIStorage() = default;

	UIElement& Get(Handle handle) { return storage[handle]; }
	const UIElement& Get(Handle handle) const { return storage[handle]; }
	Handle Add(UIElement element) { return storage.Add(element); }
	void Remove(Handle handle) { storage.Remove(handle); }

	const Storage& GetStorage() const { return storage; }

	static UIStorage& Instance() { return *instance; }

private:
	Storage storage;

	static inline std::unique_ptr<UIStorage> instance;
};

bool InitUIStorage();
void TermUIStorage();