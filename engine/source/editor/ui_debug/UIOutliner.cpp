#include "UIOutliner.h"

#ifdef EDITOR

#include "components/CompositeComponent.h"
#include "input/InputDevice.h"
#include "render/RenderSystem.h"
#include "game/GameBindings.h"
#include "scene/SceneLoader.h"
#include "editor/runtime/EditorRuntime.h"
#include "UIDebug.h"

#include "imgui.h"

#include "logs/Logs.h"

Outliner::InputContext::InputContext(Outliner& outliner)
    : outliner(&outliner)
{
}

void Outliner::InputContext::ProceedInput(InputDevice* device)
{
    if (device->IsKeyDown(Keys::Delete)) {
        outliner->DestroySelectedNode();
        return;
    }
    if (device->IsKeyHold(Keys::LeftControl) && device->IsKeyDown(Keys::S)) {
        UIDebug::SaveSceneAction();
    }
}

void Outliner::Init()
{
    inpContext = std::make_shared<InputContext>(*this);

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
    if (selectedNode.id_ >= 0 && !gSceneTree->IsValidHandle(selectedNode)) {
        ClearSelectedNode();
    }
    if (UIDebug::start_simulation) {
        return; // ignore clicks during simulation
    }

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
    if (!gSceneTree->IsValidHandle(root)) {
        root = gSceneTree->GetRoot(); // root may changed after loaded to another scene
        if (root.id_ < 0) {
            return; // no scene loaded
        }
    }
    if (!gSceneTree->IsValidHandle(selectedNode))
    {
        selectedNode = root;
        SceneTree::Entity& rootEntity = gSceneTree->Get(root);
        rootEntity.isSelected = true;
    }

    const bool isOutlinerFocused = ImGui::IsWindowFocused();
    if (isOutlinerFocused && !isFocused) {
        static_cast<EditorRuntime*>(gEngineRuntime)->GetController().SetInputContext(inpContext);
    } else if (!isOutlinerFocused && isFocused) {
        static_cast<EditorRuntime*>(gEngineRuntime)->GetController().ClearInputContext(inpContext);
    }
    isFocused = isOutlinerFocused;

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

void Outliner::DestroySelectedNode()
{
    SceneTree::Handle node = selectedNode;
    if (node.id_ == 0) {
        return; // TEMP. do not destroy scene root
    }
    if (node.id_ >= 0 && gSceneTree->IsValidHandle(node)) {
        ClearSelectedNode();
        SceneTree::Entity& entity = gSceneTree->Get(node);
        assert(entity.obj.GetRaw()); // check if entity has a valid script object attached
        SceneLoader::RemoveSpawnedComponent(node);
        GameBindings::DestroyComponent(entity.obj);
    }
}

void Outliner::ClearSelectedNode()
{
    if (selectedNode.id_ < 0) {
        return;
    }
    if (!gSceneTree->IsValidHandle(selectedNode)) {
        selectedNode = SceneTree::Handle{};
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
    flag |= (entity.children.empty()) ? ImGuiTreeNodeFlags_Leaf : 0;
    //flag |= ImGuiTreeNodeFlags_Leaf;


    if (ImGui::TreeNodeEx(entity.name.c_str(), flag))
    {
        if (ImGui::IsItemClicked() && (selectedNode != node))
        {
            SceneTree::Entity& selected = gSceneTree->Get(selectedNode);
            selected.isSelected = false;
            selectedNode = node;
            entity.isSelected = true;
        }

        for (SceneTree::Handle child : entity.children)
        {
            DrawOutlinerNode(child);
        }

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
