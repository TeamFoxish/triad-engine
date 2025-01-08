#pragma once

#ifdef EDITOR

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "scene/SceneTree.h"
#include "editor/runtime/EditorController.h"

class Component;
class CompositeComponent;


class Outliner
{
public:
	class InputContext : public EditorController::InputContextBase {
	public:
		InputContext(Outliner& outliner);

		void ProceedInput(InputDevice* device) override;

		InputTarget GetInputTarget() const override { return InputTarget::Editor; }

	private:
		Outliner* outliner;
	};

    void Init();
	//void Update(std::vector<Component*> components);
	void Update();
	void Draw();

	void DestroySelectedNode();

	const std::shared_ptr<InputContext>& GetInputContext() const { return inpContext; }

public:
	SceneTree::Handle GetRootNode() const { return root; }
	SceneTree::Handle GetSelectedNode() const { return selectedNode; }
	void ClearSelectedNode();
	static inline bool gizmo_focused = false;
private:
	void DrawOutlinerNode(SceneTree::Handle node);

	//bool Remove(std::shared_ptr<OutlinerNode> root, std::string component_to_remove);

	bool FindNodeById(SceneTree::Storage::Index entityId, SceneTree::Handle node);
	void SetSelectedNode(SceneTree::Storage::Index entityId);
	void SetSelectedNodeUpward(SceneTree::Storage::Index entityId);

private:
	SceneTree::Handle root;
	SceneTree::Handle selectedNode;

	std::shared_ptr<InputContext> inpContext;
	bool isFocused = false;
};

#endif // EDITOR