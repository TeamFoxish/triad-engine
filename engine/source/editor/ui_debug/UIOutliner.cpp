#include "UIOutliner.h"

#ifdef EDITOR

#include "components/CompositeComponent.h"
#include "input/InputDevice.h"
#include "render/RenderSystem.h"

#include "imgui.h"


void Outliner::Init(std::string root_name, std::vector<Component*> components)
{
    root = std::make_shared<OutlinerNode>();
    selectedNode = root;

    root->name = root_name;
    root->isSelected = true;

    for (Component* component : components)
    {
        //component->GetId();
        if (component->isComposite)
        {
            std::shared_ptr<OutlinerNode> ptr(CreateOutlinerNode_CC(component));
            root->children.push_back(ptr);
        }
        else
        {
            std::shared_ptr<OutlinerNode> ptr(CreateOutlinerNode_Component(component));
            root->children.push_back(ptr);
        }
    }
}

//void Outliner::Update(std::vector<Component*> components)
//{
//    std::string root_name = root->name;
//    root.reset();
//    Init(root_name, components);
//}

void Outliner::Update()
{
    if (globalInputDevice->IsKeyDown(Keys::LeftButton))
    {
        uint32_t entityId = gRenderSys->GetEntityIdUnderCursor();
        if (entityId != selectedNode->id)
        {
            SetSelectedNode(entityId);
        }
    }
}

void Outliner::Draw()
{
    ImGui::Begin("Outliner");

    ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    flag |= (root->isSelected) ? ImGuiTreeNodeFlags_Selected : 0;

    if (ImGui::TreeNodeEx(root->name.c_str(), flag))
    {
        if (ImGui::IsItemClicked() && (selectedNode->name != root->name))
        {
            selectedNode->isSelected = false;
            selectedNode = root;
            selectedNode->isSelected = true;
        }

        for (auto child : root->children)
        {
            DrawOutlinerNode(child);
        }

        ImGui::TreePop();
    }

    ImGui::End();
}

std::shared_ptr<Outliner::OutlinerNode> Outliner::GetSelectedNode() const
{
    return selectedNode;
}

Outliner::OutlinerNode* Outliner::CreateOutlinerNode_CC(Component* component)
{
    CompositeComponent* cc = dynamic_cast<CompositeComponent*>(component);

    OutlinerNode* node = new OutlinerNode;
    node->id = cc->GetId();
    node->name = cc->GetName();
    node->isSelected = false;
    node->component = component;

    for (Component* child : cc->GetChildren())
    {
        if (child->isComposite)
        {
            std::shared_ptr<OutlinerNode> ptr(CreateOutlinerNode_CC(child));
            node->children.push_back(ptr);
        }
        else
        {
            std::shared_ptr<OutlinerNode> ptr(CreateOutlinerNode_Component(child));
            node->children.push_back(ptr);
        }
    }

    return node;
}

Outliner::OutlinerNode* Outliner::CreateOutlinerNode_Component(Component* component)
{
    OutlinerNode* node = new OutlinerNode;

    node->id = component->GetId();
    node->name = component->GetName();
    node->isSelected = false;
    node->component = component;

    return node;
}

void Outliner::DrawOutlinerNode(std::shared_ptr<OutlinerNode> node)
{
    ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    flag |= (node->isSelected) ? ImGuiTreeNodeFlags_Selected : 0;
    flag |= (node->children.empty()) ? ImGuiTreeNodeFlags_Leaf : 0;

    if (ImGui::TreeNodeEx(node->name.c_str(), flag))
    {
        if (ImGui::IsItemClicked() && (selectedNode->name != node->name))
        {
            selectedNode->isSelected = false;
            selectedNode = node;
            selectedNode->isSelected = true;
        }

        for (auto child : node->children)
        {
            DrawOutlinerNode(child);
        }

        ImGui::TreePop();
    }
}

bool Outliner::FindNodeById(uint32_t entityId, std::shared_ptr<OutlinerNode> node)
{
    if (node->id == entityId)
    {
        //LOG_DEBUG("Selected node was updated. Id:{}", node->id);
        selectedNode->isSelected = false;
        selectedNode = node;
        selectedNode->isSelected = true;
        return true;
    }

    for (auto child : node->children)
    {
        if (FindNodeById(entityId, child))
        {
            return true;
        }
    }

    return false;
}

void Outliner::SetSelectedNode(uint32_t entityId)
{
    for (auto child : root->children)
    {
        if (FindNodeById(entityId, child))
        {
            break;
        }
    }
}

#endif // EDITOR