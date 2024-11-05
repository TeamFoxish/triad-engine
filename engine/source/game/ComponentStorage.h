#pragma once

#include <vector>

class Component;

class ComponentStorage {
public:
	using Components = std::vector<Component*>;

	void Term();

	void BeginUpdate();
	void EndUpdate();

	void AddComponent(Component* comp);
	void RemoveComponent(Component* comp);

	const Components& GetComponents() const { return components; }

protected:
	Components components;
	Components pendingComponents;

	bool isUpdatingComponents = false;
};
