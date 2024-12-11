#include "ComponentStorage.h"

#include "Component.h"

#include <algorithm>
#include <cassert>

void ComponentStorage::Term()
{
	for (Component* comp : components) {
		delete comp;
	}
	for (Component* comp : pendingComponents) {
		delete comp;
	}
}

void ComponentStorage::BeginUpdate()
{
	isUpdatingComponents = true;
}

void ComponentStorage::EndUpdate()
{
	isUpdatingComponents = false;
	components.insert(components.end(), pendingComponents.begin(), pendingComponents.end());
	for (Component* comp : pendingComponents) {
		idToComponent[comp->GetId()] = comp;
	}
	pendingComponents.clear();
}

void ComponentStorage::AddComponent(Component* comp)
{
#ifdef _DEBUG
	auto iter = std::find(pendingComponents.begin(), pendingComponents.end(), comp);
	assert(iter == pendingComponents.end());
	iter = std::find(components.begin(), components.end(), comp);
	assert(iter == components.end());
#endif

	if (isUpdatingComponents) {
		pendingComponents.push_back(comp);
		return;
	}
	components.push_back(comp);
	idToComponent[comp->GetId()] = comp;
}

void ComponentStorage::RemoveComponent(Component* comp)
{
	auto iter = std::find(pendingComponents.begin(), pendingComponents.end(), comp);
	if (iter != pendingComponents.end()) {
		std::iter_swap(iter, pendingComponents.end() - 1);
		pendingComponents.pop_back();
		return;
	}

	iter = std::find(components.begin(), components.end(), comp);
	if (iter != components.end()) {
		std::iter_swap(iter, components.end() - 1);
		components.pop_back();
		idToComponent.erase(comp->GetId());
		return;
	}

	assert(false);
}

Component* ComponentStorage::GetComponentById(uint32_t id) const
{
	auto iter = idToComponent.find(id);
	return iter != idToComponent.end() ? iter->second : nullptr;
}

void ComponentStorage::TEMP_AddChildComponentIdEntry(Component* comp)
{
	idToComponent[comp->GetId()] = comp;
}
