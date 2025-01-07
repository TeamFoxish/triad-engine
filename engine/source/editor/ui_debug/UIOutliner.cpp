#include "UIOutliner.h"

#ifdef EDITOR

#include "components/CompositeComponent.h"
#include "input/InputDevice.h"
#include "render/RenderSystem.h"

#include "imgui.h"

#include "logs/Logs.h"

void Outliner::Init()
{
    root = gSceneTree->GetRoot();
    if (root.id_ < 0) {
        return; // scene empty
    }
    selectedNode = root;
    SceneTree::Entity& rootEntity = gSceneTree->Get(root);
    rootEntity.isSelected = true;
}

//void Outliner::Update(std::vector<Component*> components)
//{
//    std::string root_name = root->name;
//    root.reset();
//    Init(root_name, components);
//}

void Outliner::Update()
{
    static int clicksCount = 0;
    const int newClicksCount = ImGui::GetMouseClickedCount(ImGuiMouseButton_Left);
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        gizmo_focused = false;
        int32_t entityId = gRenderSys->GetEntityIdUnderCursor();
        if (entityId >= 0 && entityId != selectedNode.id_)
        {
            SetSelectedNode(entityId);
            gizmo_focused = true;
        }
    } 
    else if (clicksCount == 1 && newClicksCount == 0) {
        // delayed single left mouse click
        gizmo_focused = false;
        int32_t entityId = gRenderSys->GetEntityIdUnderCursor();
        if (entityId >= 0)
        {
            SetSelectedNodeUpward(entityId);
            gizmo_focused = true;
        }
    }
    clicksCount = newClicksCount;
}

void Outliner::Draw()
{
    ImGui::Begin("Outliner");
    if (root.id_ < 0) {
        ImGui::End();
        return;
    }

    SceneTree::Entity& rootEntity = gSceneTree->Get(root);
    assert(rootEntity.isComposite);

    ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    flag |= (rootEntity.isSelected) ? ImGuiTreeNodeFlags_Selected : 0;

    //if (ImGui::TreeNodeEx(rootEntity.name.c_str(), flag))
    if (ImGui::TreeNodeEx("Scene", flag))
    {
        if (ImGui::IsItemClicked() && (selectedNode != root))
        {
            SceneTree::Entity& selected = gSceneTree->Get(selectedNode);
            selected.isSelected = false;
            selectedNode = root;
            rootEntity.isSelected = true;
        }

        for (SceneTree::Handle child : rootEntity.children)
        {
            DrawOutlinerNode(child);
        }

        ImGui::TreePop();
    }

    ImGui::End();
}

void Outliner::ClearSelectedNode()
{
    if (selectedNode.id_ < 0) {
        return;
    }
    SceneTree::Entity& entity = gSceneTree->Get(selectedNode);
    entity.isSelected = false;
    selectedNode = SceneTree::Handle{};
}

void Outliner::DrawOutlinerNode(SceneTree::Handle node)
{
    SceneTree::Entity& entity = gSceneTree->Get(node);
    if (!entity.isComposite) {
        return;
    }

    ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    flag |= (entity.isSelected) ? ImGuiTreeNodeFlags_Selected : 0;

    // ToDo: will outliner draw only components without their children(?)
    //flag |= (entity.children.empty()) ? ImGuiTreeNodeFlags_Leaf : 0;
    flag |= ImGuiTreeNodeFlags_Leaf;
    

    if (ImGui::TreeNodeEx(entity.name.c_str(), flag))
    {
        if (ImGui::IsItemClicked() && (selectedNode != node))
        {
            SceneTree::Entity& selected = gSceneTree->Get(selectedNode);
            selected.isSelected = false;
            selectedNode = node;
            entity.isSelected = true;
        }

        // TEMP
        /*for (SceneTree::Handle child : entity.children)
        {
            DrawOutlinerNode(child);
        }*/

        ImGui::TreePop();
    }
}

bool Outliner::FindNodeById(SceneTree::Storage::Index entityId, SceneTree::Handle node)
{
    SceneTree::Entity& entity = gSceneTree->Get(node);
    if (node.id_ == entityId)
    {
        //LOG_DEBUG("Selected node was updated. Id:{}", node->id);
        SceneTree::Entity& selected = gSceneTree->Get(selectedNode);
        selected.isSelected = false;
        selectedNode = node;
        entity.isSelected = true;
        return true;
    }

    for (SceneTree::Handle childHandle : entity.children)
    {
        if (FindNodeById(entityId, childHandle))
        {
            return true;
        }
    }

    return false;
}

void Outliner::SetSelectedNode(SceneTree::Storage::Index entityId)
{
    SceneTree::Handle handle = gSceneTree->GetHandleFromId(entityId);
    if (handle.id_ < 0 || FindNodeById(entityId, handle)) {
        return;
    }

    SceneTree::Entity& entity = gSceneTree->Get(handle);
    for (SceneTree::Handle childHandle : entity.children)
    {
        if (FindNodeById(entityId, childHandle))
        {
            break;
        }
    }
}

void Outliner::SetSelectedNodeUpward(SceneTree::Storage::Index entityId)
{
    SceneTree::Handle handle = gSceneTree->GetHandleFromId(entityId);
    if (handle.id_ < 0) {
        return;
    }
    SceneTree::Entity& entity = gSceneTree->Get(handle);
    if (entity.parent.id_ < 0 || entity.parent == selectedNode) {
        return;
    }
    if (selectedNode.id_ >= 0) {
        SceneTree::Entity& selected = gSceneTree->Get(selectedNode);
        selected.isSelected = false;
    }
    selectedNode = entity.parent;
    SceneTree::Entity& parent = gSceneTree->Get(entity.parent);
    parent.isSelected = true;
}

#endif // EDITOR