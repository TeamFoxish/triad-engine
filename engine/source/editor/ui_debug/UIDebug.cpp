#include "UIDebug.h"

#include <filesystem>
#include <windows.h>
#include <WinUser.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include "ImGuizmo.h"

#include "runtime/EngineRuntime.h"
#include "render/RenderSystem.h"
#include "render/Renderer.h"
#include "os/wnd.h"
#include "os/Window.h"

#include "runtime/EngineRuntime.h"
#include "game/Game.h"
#include "scene/Scene.h"
#include "scene/SceneLoader.h"
#include "scene/SceneBindings.h"
#include "scripts/ComponentLoader.h"
#include "components/CompositeComponent.h"
#include "game/ComponentStorage.h"
#include "components/EditorCamera.h"
#include "shared/SharedStorage.h"
#include "scripts/ScriptSystem.h"
#include "scripts/ScriptObject.h"
#include "input/InputDevice.h"
#include "resource/ResourceSystem.h"
#include "navigation/NavMeshSystem.h"
#include "navigation/NavMeshResources.h"

#include "logs/Logs.h"
#include "shared/Shared.h"

#ifdef EDITOR
#include "editor/runtime/EditorRuntime.h"
#include "EditorInspector.h"

static std::unique_ptr<EditorInspector> inspector;
#endif // EDITOR

#include <game/PrefabLoader.h>

#define DRAG_SPEED 0.01f

static constexpr float DegreesDragSpeed = Math::RadToDeg(DRAG_SPEED);


void UIDebug::Init(Window* window)
{
    isInitted = true;

#ifdef EDITOR
    viewportInpContext = std::make_shared<ViewportInputContext>();
#endif

    // TODO: add flags to enable/disable docking and viewporting
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    // Setup ImGui Platform/Render backends


    ImGui_ImplWin32_Init(wndGetHWND(window));
    ImGui_ImplDX11_Init(gRenderSys->GetRenderer()->GetDevice(), gRenderSys->GetRenderer()->GetDeviceContext());

    if (std::filesystem::exists("DefaultImGuiSettings.ini") && useDefaultSettings)
    {
        ImGui::LoadIniSettingsFromDisk("DefaultImGuiSettings.ini");
    }

#ifdef EDITOR
    outliner.Init();
    contentBrowser.Init(&static_cast<EditorRuntime*>(gEngineRuntime)->assetManager);
#endif // EDITOR
}

void UIDebug::StartNewFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

// TODO: move somewhere
static std::string bName = "Start";
static float pos1[3] = {0, 0, 0};
static float pos2[3] = {0, 0, 0};
static ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;

void UIDebug::TestDraw()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
#ifdef EDITOR
    {
        // Docking space
        {
            const ImVec2 wSize{
                (float)gRenderSys->GetRenderer()->GetWindow()->GetBorderedWidth(),
                (float)gRenderSys->GetRenderer()->GetWindow()->GetBorderedHeight()
            };
            const ImVec2 wWorkSize{
                (float)gRenderSys->GetRenderer()->GetWindow()->GetClientWidth(),
                (float)gRenderSys->GetRenderer()->GetWindow()->GetClientHeight() - 20
            };
            ImGuiViewport* imViewPort = ImGui::GetMainViewport();
            imViewPort->Size = wSize;
            imViewPort->WorkSize = wWorkSize;
            ImGui::DockSpaceOverViewport(0, imViewPort);
        }

        {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    //bool isSelected = false;
                    if (start_simulation) {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::MenuItem("Save scene"))
                    {
                        SaveSceneAction();
                    }
                    if (start_simulation) {
                        ImGui::EndDisabled();
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Assets"))
                {
                    if (ImGui::BeginMenu("Add Component"))
                    {
                        Triad::FileIO::FPath componentPath = "./assets/components";
                        Triad::FileIO::FPath assetsPath = "./assets";

                        Triad::FileIO::IterateDirectory(
                            componentPath,
                            [assetsPath, componentPath] (YAML::Node scriptDesc, const Triad::FileIO::FPath path) {
                                std::string fileName = std::filesystem::relative(path, componentPath).stem().string();
                                std::string filePath = std::filesystem::relative(path, assetsPath).generic_string();

                                if (ImGui::MenuItem(fileName.c_str()))
                                {
                                    // Add prefab to the scene
                                    ResTag tag = ResTag(ToStrid("res://" + filePath));
                                    gResourceSys->LoadResource(tag);

                                    SceneTree::Entity& rootEntity = gSceneTree->Get(gSceneTree->GetRoot());
                                    std::optional<YAML::Node> sceneDesc = SceneLoader::FindSpawnedComponent(rootEntity.obj);
                                    if (sceneDesc) {
                                        // TODO: delete obj ptr
                                        ScriptObject* obj = ComponentLoader::CreateComponent(tag, &rootEntity.obj);
                                        const SceneTree::Handle entHandle = GetEntityHandleFromScriptObject(obj->GetRaw());
                                        if (entHandle != SceneTree::Handle{}) {
                                            asQWORD entId = GetEntityIdFromHandle(entHandle);
                                            const std::string fName = Triad::FileIO::FPath(filePath).filename().replace_extension().string();
                                            std::string entName = std::format("{}_{}", fName, entId);
                                            while ((*sceneDesc)["objects"][entName]) {
                                                entName = std::format("{}_{}", fName, ++entId); // TEMP
                                            }
                                            ComponentLoader::SetComponentName(*obj, entName);
                                            YAML::Node& compNode = (*sceneDesc)["objects"][entName] = YAML::Node();
                                            ComponentLoader::PopulateEmptySceneYaml(compNode, tag);
                                            SceneLoader::AddSpawnedComponent(*obj, compNode);
                                        }
                                    }
                                }
                            },
                            ".component"
                        );

                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Add Prefab"))
                    {
                        Triad::FileIO::FPath prefabPath = "./assets/prefabs";
                        Triad::FileIO::FPath assetsPath = "./assets";
                        Triad::FileIO::IterateDirectory(
                            prefabPath,
                            [assetsPath, prefabPath](YAML::Node scriptDesc, const Triad::FileIO::FPath path) {
                                std::string fileName = std::filesystem::relative(path, prefabPath).stem().string();
                                std::string filePath = std::filesystem::relative(path, assetsPath).generic_string();
                                
                                if (ImGui::MenuItem(fileName.c_str()))
                                {
                                    // Add prefab to the scene
                                    ResTag tag = ResTag(ToStrid("res://" + filePath));
                                    gResourceSys->LoadResource(tag);

                                    SceneTree::Entity& rootEntity = gSceneTree->Get(gSceneTree->GetRoot());
                                    std::optional<YAML::Node> sceneDesc = SceneLoader::FindSpawnedComponent(rootEntity.obj);
                                    if (sceneDesc) {
                                        // TODO: delete obj ptr
                                        ScriptObject* obj = PrefabLoader::Create(tag, &rootEntity.obj);
                                        const SceneTree::Handle entHandle = GetEntityHandleFromScriptObject(obj->GetRaw());
                                        if (entHandle != SceneTree::Handle{}) {
                                            asQWORD entId = GetEntityIdFromHandle(entHandle);
                                            const std::string fName = Triad::FileIO::FPath(filePath).filename().replace_extension().string();
                                            std::string entName = std::format("{}_{}", fName, entId);
                                            while ((*sceneDesc)["objects"][entName]) {
                                                entName = std::format("{}_{}", fName, ++entId); // TEMP
                                            }
                                            YAML::Node& compNode = (*sceneDesc)["objects"][entName] = YAML::Node();
                                            PrefabLoader::PopulateEmptySceneYaml(compNode, tag);
                                            SceneLoader::AddSpawnedComponent(*obj, compNode);
                                        }
                                    }
                                }
                            },
                            ".prefab"
                        );

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }
        }

        // Outliner
        {
            static SceneTree::Handle prevSelected = outliner.GetSelectedNode();
            if (!ImGuizmo::IsOver()) {
                outliner.Update();
            }
            outliner.Draw();
            const SceneTree::Handle selectedNode = outliner.GetSelectedNode();
            if (selectedNode != prevSelected) {
                // change inspector view
                if (selectedNode == SceneTree::Handle{}) {
                    inspector.reset();
                } else {
                    inspector = std::make_unique<EditorInspector>(selectedNode);
                }
                prevSelected = selectedNode;
            }
        }

        // File system
        {
            ImGui::Begin("File system", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
            // ToDo: change position of simulation button
            if (ImGui::Button(bName.c_str()))
            {
                start_simulation = !start_simulation;
                bName = start_simulation ? "Pause" : "Start";
                if (start_simulation) {
                    onSimulationStart.Broadcast();
                }
                else {
                    onSimulationEnd.Broadcast();
                }
            }
            ImGui::End();
            contentBrowser.Draw();
        }

        // Script debugger
        {
            ImGui::Begin("Debugger", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
            // ToDo: change position of simulation button
            if (ImGui::Button("Debug menu"))
            {
                gScriptSys->InvokeDebuggerMenu();
            }
            ImGui::End();
        }

        // Nav Mesh Generation
        {
            BuildConfig& config = gNavigation->GetBuilder().GetCurrentConfig();

            ImGui::Begin("Navigation", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
            ImGui::Text("Rasterization");
            ImGui::SliderFloat("Cell Size", &config.rasterization.cellSize, 0.1f, 1.0f);
            ImGui::SliderFloat("Cell Height", &config.rasterization.cellHeigth, 0.1f, 1.0f);

            ImGui::Separator();

            static bool isAgentLoaded = false;
            static NavMeshAgent agent;
            ImGui::Text("Agent");

            static std::string resInp = "res://navmeshagent/Base.agent";
            ImGui::InputText("Agent Resource", &resInp);
            if (!resInp.empty()) {
                ResTag agentTag = ToStrid(resInp);
                if (!gResourceSys->IsResourceLoaded(agentTag)) {
                    gResourceSys->LoadResource(agentTag);
                }
                const auto iter = NavMeshResources::Instance().agents.find(agentTag);
                if (iter != NavMeshResources::Instance().agents.end()) {
                    if (!isAgentLoaded) {
                        agent = iter->second;
                        isAgentLoaded = true;
                    }
                    ImGui::SliderFloat("Height", &agent.height, 0.1f, 5.0f);
                    ImGui::SliderFloat("Radius", &agent.radius, 0.0f, 5.0f);
                    ImGui::SliderFloat("Max Climb", &agent.maxClimb, 0.1f, 5.0f);
                    ImGui::SliderFloat("Max Slope", &agent.maxSlope, 0.0f, 90.0f);
                }
            }

            ImGui::Separator();
            ImGui::Text("Region");
            ImGui::SliderFloat("Min Region Size", &config.region.minRegionSize, 0.0f, 150.0f);
            ImGui::SliderFloat("Merged Region Size", &config.region.mergedRegionSize, 0.0f, 150.0f);

            // ImGui::Separator();
            // ImGui::Text("Partitioning", nullptr);
            // if (ImGui::Checkbox("Watershed", m_partitionType == SAMPLE_PARTITION_WATERSHED))
            //     m_partitionType = SAMPLE_PARTITION_WATERSHED;
            // if (ImGui::Checkbox("Monotone", m_partitionType == SAMPLE_PARTITION_MONOTONE))
            //     m_partitionType = SAMPLE_PARTITION_MONOTONE;
            // if (ImGui::Checkbox("Layers", m_partitionType == SAMPLE_PARTITION_LAYERS))
            //     m_partitionType = SAMPLE_PARTITION_LAYERS;

            ImGui::Separator();
            ImGui::Text("Filtering");
            ImGui::Checkbox("Low Hanging Obstacles", &config.filterLowHangingObstacles);
            ImGui::Checkbox("Ledge Spans", &config.filterLedgeSpans);
            ImGui::Checkbox("Walkable Low Height Spans", &config.filterWalkableLowHeightSpans);

            ImGui::Separator();
            ImGui::Text("Polygonization");
            ImGui::SliderFloat("Max Edge Length", &config.polygonization.maxEdgeLength, 0.0f, 50.0f);
            ImGui::SliderFloat("Max Edge Error", &config.polygonization.maxEdgeError, 0.1f, 3.0f);
            ImGui::SliderFloat("Verts Per Poly", &config.polygonization.vertsPerPoly, 3.0f, 12.0f);		

            ImGui::Separator();
            ImGui::Text("Detail Mesh");
            ImGui::SliderFloat("Sample Distance", &config.detail.sampleDistance, 0.0f, 16.0f);
            ImGui::SliderFloat("Max Sample Error", &config.detail.maxSampleError, 0.0f, 16.0f);
	
	        ImGui::Separator();
            if(ImGui::Button("Build"))
            {
                gNavigation->Build(config, &agent);
            }
            
            ImGui::Separator();
            ImGui::InputFloat3("Start Position", pos1);
            ImGui::InputFloat3("End Position", pos2);
            if(ImGui::Button("Test Path"))
            {
                gNavigation->GenerateTestPath(&agent, pos1, pos2);
            }
            ImGui::End();
        }


        // Viewport in window
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Scene Test", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::GetCurrentWindow();
            UpdateViewportPos();
            if (HandleViewportResize()) {
                ImGui::Image((ImTextureID)(intptr_t)gRenderSys->GetRenderer()->GetColorPassSrt(), ImGui::GetWindowSize());

                const bool isFocused = ImGui::IsWindowFocused();
                if (isFocused && !isSceneFocused) {
                    static_cast<EditorRuntime*>(gEngineRuntime)->GetController().SetInputContext(viewportInpContext);
                } else if (!isFocused && isSceneFocused) {
                    static_cast<EditorRuntime*>(gEngineRuntime)->GetController().ClearInputContext(viewportInpContext);
                }
                isSceneFocused = isFocused;
            }

            DrawGizmo();

            ImGui::End();
            ImGui::PopStyleVar();
        }

        //ImGui::SetNextWindowSize(ImGui::GetWindowSize());
        bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
    }
#endif // EDITOR
}

void UIDebug::Render()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void UIDebug::Destroy()
{
    isInitted = false;

#ifdef EDITOR
    viewportInpContext.reset();
    inspector.reset();
#endif

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool UIDebug::GetUIDebugFlag()
{
    return isInitted && !isSceneFocused;
}

void UIDebug::UpdateViewportPos()
{
    const ImVec2 windowPos = ImGui::GetMainViewport()->Pos;
    const ImVec2 viewportPos = ImGui::GetWindowPos();
    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();
    vMin.x += viewportPos.x;
    vMin.y += viewportPos.y;
    vMax.x += viewportPos.x;
    vMax.y += viewportPos.y;
    viewportX = (int)(vMin.x - windowPos.x);
    viewportY = (int)(vMin.y - windowPos.y);
}

bool UIDebug::HandleViewportResize()
{
    ImVec2 view = ImGui::GetContentRegionAvail();

    if (view.x != viewportWidth || view.y != viewportHeight)
    {
        if (view.x == 0 || view.y == 0)
        {
            // The window is too small or collapsed.
            return false;
        }
        if (viewportWidth == -1 && viewportHeight == -1) {
            viewportWidth = view.x;
            viewportHeight = view.y;
            return true;
        }

        viewportWidth = view.x;
        viewportHeight = view.y;

        // TODO: resize only when resize complete
        // buffers should resize next frame
        gViewportResized.Broadcast(viewportWidth, viewportHeight);

        // The window state has been successfully changed.
        return true;
    }

    // The window state has not changed.
    return true;
}

#ifdef EDITOR
static void DrawVec3Control(const std::string& label, Math::Vector3& values, float dragSpeed, float min = -100.f, float max = 100.f, ImGuiSliderFlags flag = 0, float columnWidth = 100.f)
{
    ImGui::PushID(label.c_str());

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text(label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
    ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    if (ImGui::Button("X", buttonSize)) {}
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X", &values.x, dragSpeed, min, max, "%.3f", flag);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
    if (ImGui::Button("Y", buttonSize)) {}
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values.y, dragSpeed, min, max, "%.3f", flag);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    if (ImGui::Button("Z", buttonSize)) {}
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values.z, dragSpeed, min, max, "%.3f", flag);
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();

    ImGui::Columns(1);

    ImGui::PopID();
}

void UIDebug::DrawGizmo()
{
    //gizmoSelected = ImGuizmo::IsOver();
    auto node = outliner.GetSelectedNode();

    if (node.id_ >= 0 && node != outliner.GetRootNode())
    {
        enum class GizmoSpace : int {
            World = 0,
            Local
        };
        static int gizmoSpace = (int)GizmoSpace::World;

        SceneTree::Entity& entity = gSceneTree->Get(node);
        if (entity.isComposite && entity.transform.id_ >= 0) {
            Math::Transform& trs = SharedStorage::Instance().transforms.AccessWrite(entity.transform);

            // ImGui window
            {
                ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

                ImGui::SeparatorText("Transform Manually");

                if (ImGui::Button("Translate"))
                {
                    operation = ImGuizmo::OPERATION::TRANSLATE;
                }
                ImGui::SameLine();

                if (ImGui::Button("Rotate"))
                {
                    operation = ImGuizmo::OPERATION::ROTATE;
                }
                ImGui::SameLine();

                if (ImGui::Button("Scale"))
                {
                    operation = ImGuizmo::OPERATION::SCALE;
                }
                ImGui::SameLine();

                ImGui::RadioButton("World", &gizmoSpace, (int)GizmoSpace::World);
                ImGui::SameLine();
                ImGui::RadioButton("Local", &gizmoSpace, (int)GizmoSpace::Local);

                ImGui::SeparatorText("");

                bool transformChanged = false;

                // Position
                const Math::Vector3 pos = trs.GetLocalPosition();
                Math::Vector3 newPos = pos;
                DrawVec3Control("Position", newPos, DRAG_SPEED);
                if (Math::Vector3::DistanceSquared(newPos, pos) > Math::Epsilon) {
                    trs.SetLocalPosition(newPos);
                    transformChanged = true;
                }

                // Rotation
                Math::Quaternion rot = trs.GetLocalRotation();
                const Math::Vector3 euler = Math::RadToDeg(rot.ToEuler());
                Math::Vector3 changed = euler;
                DrawVec3Control("Rotation", changed, DegreesDragSpeed, -360.0f, 360.0f, ImGuiSliderFlags_WrapAround);

                const Math::Vector3 delta = Math::DegToRad(changed - euler);
                if (delta.LengthSquared() > Math::Epsilon) {
                    rot *= Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitY, delta.y);
                    rot *= Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitX, delta.x);
                    rot *= Math::Quaternion::CreateFromAxisAngle(Math::Vector3::UnitZ, delta.z);
                    trs.SetLocalRotation(rot);
                    transformChanged = true;
                }

                // Scale
                const Math::Vector3 scale = trs.GetLocalScale();
                Math::Vector3 newScale = scale;
                DrawVec3Control("Scale", newScale, DRAG_SPEED, 0.f, 100.f);
                if (Math::Vector3::DistanceSquared(newScale, scale) > Math::Epsilon) {
                    trs.SetLocalScale(newScale);
                    transformChanged = true;
                }

                if (transformChanged) {
                    SceneLoader::UpdateSpawnedComponentTransform(node);
                }

                //DrawAdditionalFields(&entity.obj);
                if (inspector) {
                    inspector->Draw();
                }

                ImGui::End();
            }

            if (start_simulation) {
                return;
            }

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            float w_width = (float)ImGui::GetWindowWidth();
            float w_height = (float)ImGui::GetWindowHeight();
            const float headerHeight = ImGui::GetTextLineHeightWithSpacing(); // https://github.com/CedricGuillemet/ImGuizmo/issues/109
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + headerHeight, w_width, w_height);

            EditorCamera* camera = gTempGame->GetEditorCamera();

            // View
            const Math::Matrix& viewMatrix = camera->GetViewMatrix();

            // Projection
            const Math::Matrix& projectionMatrix = camera->GetProjectionMatrix();

            // Transform
            //Math::Transform& trs = SharedStorage::Instance().transforms.AccessWrite(entity.transform);
            Math::Matrix matr = trs.GetMatrix(); // TODO: check if gizmo works for child entities
            Math::Matrix matrTemp = matr;

            if (ImGuizmo::Manipulate((float*)viewMatrix.m, (float*)projectionMatrix.m, operation, gizmoSpace == (int)GizmoSpace::World ? ImGuizmo::WORLD : ImGuizmo::LOCAL, (float*)matr.m)) {
                trs.SetMatrix(matr);
                SceneLoader::UpdateSpawnedComponentTransform(node);
            }
        }
    }
}

void UIDebug::ViewportInputContext::ProceedInput(InputDevice* device)
{
    if (device->IsKeyDown(Keys::Delete)) {
        outliner.DestroySelectedNode();
        return;
    }
    if (device->IsKeyHold(Keys::LeftControl) && device->IsKeyDown(Keys::S)) {
        UIDebug::SaveSceneAction();
    }
}

void UIDebug::DrawAdditionalFields(ScriptObject* obj)
{
    void* currentValuePointer;
    int fieldType;

    for (asUINT prop = 0; prop < obj->GetRaw()->GetPropertyCount(); ++prop)
    {
        std::string name = obj->GetRaw()->GetPropertyName(prop);
        currentValuePointer = obj->GetRaw()->GetAddressOfProperty(prop);
        fieldType = obj->GetRaw()->GetPropertyTypeId(prop);

        switch (fieldType)
        {
        case asTYPEID_BOOL:
        {
            bool* currentValueBool = static_cast<bool*>(currentValuePointer);
            ImGui::Checkbox(name.c_str(), currentValueBool);
            break;
        }

        case asTYPEID_INT8:
        {
            int8_t min = -100;
            int8_t max = 100;

            int8_t* currentValueInt8 = static_cast<int8_t*>(currentValuePointer);
            ImGui::DragInt(name.c_str(), (int*)currentValueInt8, 1.F,
                min,
                max
            );
            break;
        }

        case asTYPEID_INT16:
        {
            int16_t min = -1000;
            int16_t max = 1000;

            int16_t* currentValueInt16 = static_cast<int16_t*>(currentValuePointer);
            ImGui::DragInt(name.c_str(), (int*)currentValueInt16, 1.F,
                min,
                max
            );
            break;
        }

        case asTYPEID_INT32:
        {
            int32_t min = -1000;
            int32_t max = 1000;

            int32_t* currentValueInt32 = static_cast<int32_t*>(currentValuePointer);
            ImGui::DragInt(name.c_str(), (int*)currentValueInt32, 1.F,
                min,
                max
            );
            break;
        }

        case asTYPEID_INT64:
        {
            int64_t min = -1000;
            int64_t max = 1000;

            int64_t* currentValueInt64 = static_cast<int64_t*>(currentValuePointer);
            ImGui::DragInt(name.c_str(), (int*)currentValueInt64, 1.F,
                min,
                max
            );
            break;
        }

        case asTYPEID_UINT8:
        {
            uint8_t min = 0;
            uint8_t max = 100;

            uint8_t* currentValueInt8 = static_cast<uint8_t*>(currentValuePointer);
            ImGui::DragInt(name.c_str(), (int*)currentValueInt8, 1.F,
                min,
                max
            );
            break;
        }

        case asTYPEID_UINT16:
        {
            uint16_t min = 0;
            uint16_t max = 1000;

            uint16_t* currentValueInt16 = static_cast<uint16_t*>(currentValuePointer);
            ImGui::DragInt(name.c_str(), (int*)currentValueInt16, 1.F,
                min,
                max
            );
            break;
        }

        case asTYPEID_UINT32:
        {
            uint32_t min = 0;
            uint32_t max = 1000;

            uint32_t* currentValueInt32 = static_cast<uint32_t*>(currentValuePointer);
            ImGui::DragInt(name.c_str(), (int*)currentValueInt32, 1.F,
                min,
                max
            );
            break;
        }

        case asTYPEID_UINT64:
        {
            uint64_t min = 0;
            uint64_t max = 1000;

            uint64_t* currentValueInt64 = static_cast<uint64_t*>(currentValuePointer);
            ImGui::DragInt(name.c_str(), (int*)currentValueInt64, 1.F,
                min,
                max
            );
            break;
        }

        case asTYPEID_FLOAT:
        {
            float min = -100.f;
            float max = 100.f;

            float* currentValueFloat = static_cast<float*>(currentValuePointer);
            ImGui::DragFloat(name.c_str(), currentValueFloat, 0.01f,
                min,
                max
            );

            break;
        }
        case asTYPEID_DOUBLE:
        {
            double min = -100.f;
            double max = 100.f;

            double* currentValueDouble = static_cast<double*>(currentValuePointer);
            ImGui::DragFloat(name.c_str(), (float*)currentValueDouble, 0.01f,
                min,
                max
            );
            break;
        }

        case asTYPEID_SCRIPTOBJECT:
        {
            ImGui::SeparatorText(name.c_str());

            ScriptObject* currentValueScriptObject = static_cast<ScriptObject*>(currentValuePointer);
            DrawAdditionalFields(currentValueScriptObject);

            break;
        }
        case asTYPEID_APPOBJECT:
        {
            ImGui::SeparatorText(("App Object: " + name).c_str());

            CNativeObject** currentValueNativeObject = static_cast<CNativeObject**>(currentValuePointer);

            break;
        }

        case asTYPEID_VOID:
        default:
        {
            currentValuePointer = nullptr;
            break;
        }
        }
    }
}

void UIDebug::SaveSceneAction()
{
    const SceneTree::Handle sceneRoot = gSceneTree->GetRoot();
    if (!gSceneTree->IsValidHandle(sceneRoot)) {
        LOG_ERROR("failed to save scene. no active scene was found at root");
        return;
    }
    SceneLoader::SaveScene(sceneRoot);
}
#endif // EDITOR
