#pragma once

#include "game/ComponentStorage.h"

class Scene {
public:
	Scene() = default;
	~Scene() = default;

	ComponentStorage& GetStorage() { return storage; }
	const ComponentStorage::Components& GetComponents() const { return storage.GetComponents(); }

protected:
	ComponentStorage storage;
};
