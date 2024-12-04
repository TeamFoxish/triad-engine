#pragma once

#ifdef EDITOR

#include <iostream>
#include <string>
#include <vector>
#include <memory>

class Component;
class CompositeComponent;


class Outliner
{
public:
    void Init(std::string root_name, std::vector<Component*> components);
	//void Update(std::vector<Component*> components);
	void Update();
	void Draw();


private:
	struct OutlinerNode
	{
		uint32_t id = -1;
		std::string name = "empty node";
		bool isSelected = false;
		std::vector<std::shared_ptr<OutlinerNode>> children;

		Component* component = nullptr; // ToDo: redo outliner node
	};

public:
	std::shared_ptr<OutlinerNode> GetSelectedNode() const;

private:
	OutlinerNode* CreateOutlinerNode_CC(Component* component);
	OutlinerNode* CreateOutlinerNode_Component(Component* component);

	void DrawOutlinerNode(std::shared_ptr<OutlinerNode> node);

	bool Remove(std::shared_ptr<OutlinerNode> root, std::string component_to_remove);

	bool FindNodeById(uint32_t entityId, std::shared_ptr<OutlinerNode> node);
	void SetSelectedNode(uint32_t entityId);

private:
	std::shared_ptr<OutlinerNode> root;
	std::shared_ptr<OutlinerNode> selectedNode;
};

#endif // EDITOR