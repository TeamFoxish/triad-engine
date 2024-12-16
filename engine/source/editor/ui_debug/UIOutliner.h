#pragma once

#ifdef EDITOR

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "scene/SceneTree.h"

class Component;
class CompositeComponent;


class Outliner
{
public:
    void Init();
	//void Update(std::vector<Component*> components);
	void Update();
	void Draw();

public:
	SceneTree::Handle GetRootNode() const { return root; }
	SceneTree::Handle GetSelectedNode() const { return selectedNode; }

private:
	void DrawOutlinerNode(SceneTree::Handle node);

	//bool Remove(std::shared_ptr<OutlinerNode> root, std::string component_to_remove);

	bool FindNodeById(uint32_t entityId, SceneTree::Handle node);
	void SetSelectedNode(uint32_t entityId);

private:
	SceneTree::Handle root;
	SceneTree::Handle selectedNode;
};

#endif // EDITOR