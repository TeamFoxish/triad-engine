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
	void Update(std::vector<Component*> components);
	void Draw();

private:
	struct OutlinerNode
	{
		std::string name = "empty node";
		bool isSelected = false;
		std::vector<std::shared_ptr<OutlinerNode>> children;
	};

private:
	OutlinerNode* CreateOutlinerNode_CC(CompositeComponent* cc);
	OutlinerNode* CreateOutlinerNode_Component(Component* component);

	void DrawOutlinerNode(std::shared_ptr<OutlinerNode> node);

	bool Remove(std::shared_ptr<OutlinerNode> root, std::string component_to_remove);

private:
	std::shared_ptr<OutlinerNode> root;
	std::shared_ptr<OutlinerNode> selectedNode;
};

#endif // EDITOR