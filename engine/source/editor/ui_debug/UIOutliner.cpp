#include "UIOutliner.h"

#ifdef EDITOR

#include "components/CompositeComponent.h"
#include "imgui.h"


void Outliner::Init(std::string root_name, std::vector<Component*> components)
{
    root = std::make_shared<OutlinerNode>();
    selectedNode = root;

    root->name = root_name;
    root->isSelected = true;

    for (Component* component : components)
    {
        if (component->isComposite)
        {
            CompositeComponent* cc = dynamic_cast<CompositeComponent*>(component);
            std::shared_ptr<OutlinerNode> ptr(CreateOutlinerNode_CC(cc));
            root->children.push_back(ptr);
        }
        else
        {
            std::shared_ptr<OutlinerNode> ptr(CreateOutlinerNode_Component(component));
            root->children.push_back(ptr);
        }
    }
}

void Outliner::Update(std::vector<Component*> components)
{
    std::string root_name = root->name;
    root.reset();
    Init(root_name, components);
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

Outliner::OutlinerNode* Outliner::CreateOutlinerNode_CC(CompositeComponent* cc)
{
    OutlinerNode* node = new OutlinerNode;
    node->name = cc->GetName();
    node->isSelected = false;

    for (Component* child : cc->GetChildren())
    {
        if (child->isComposite)
        {
            CompositeComponent* cc = dynamic_cast<CompositeComponent*>(child);
            std::shared_ptr<OutlinerNode> ptr(CreateOutlinerNode_CC(cc));
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

    node->name = component->GetName();
    node->isSelected = false;

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

#endif // EDITOR