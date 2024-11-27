#pragma once

#include <vector>
#include <unordered_map>

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

	Component* GetComponentById(uint32_t id) const;

	void TEMP_AddChildComponentIdEntry(Component* comp);

protected:
	Components components;
	Components pendingComponents;
	std::unordered_map<uint32_t, Component*> idToComponent;

	bool isUpdatingComponents = false;
};
