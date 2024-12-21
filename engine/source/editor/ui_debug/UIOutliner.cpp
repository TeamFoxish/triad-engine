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
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        uint32_t entityId = gRenderSys->GetEntityIdUnderCursor();
        union ConvHelper {
            uint32_t orig = 0;
            decltype(selectedNode.id_) target;
        } helper;
        helper.orig = entityId;
        if (helper.target != selectedNode.id_) // TODO: check if actually works (note: entity ids are not set in renderables currently)
        {
            SetSelectedNode(entityId);
        }
        gizmo_focused = true;
    }
    // ToDo: remove camera jerk when out of gizmo focus
    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        gizmo_focused = false;
    }
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

bool Outliner::FindNodeById(uint32_t entityId, SceneTree::Handle node)
{
    //if (node->id == entityId)
    //{
    //    //LOG_DEBUG("Selected node was updated. Id:{}", node->id);
    //    selectedNode->isSelected = false;
    //    selectedNode = node;
    //    selectedNode->isSelected = true;
    //    return true;
    //}

    //for (auto child : node->children)
    //{
    //    if (FindNodeById(entityId, child))
    //    {
    //        return true;
    //    }
    //}

    //return false;
    return true;
}

void Outliner::SetSelectedNode(uint32_t entityId)
{
    /*for (auto child : root->children)
    {
        if (FindNodeById(entityId, child))
        {
            break;
        }
    }*/
}

#endif // EDITOR