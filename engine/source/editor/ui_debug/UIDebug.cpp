#include "UIDebug.h"

#include <filesystem>
#include <windows.h>
#include <WinUser.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "imgui.h"
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
#include "components/CompositeComponent.h"
#include "game/ComponentStorage.h"
#include "components/EditorCamera.h"
#include "shared/SharedStorage.h"
#include "scripts/ScriptSystem.h"
#include "scripts/ScriptObject.h"

#include "logs/Logs.h"

#define DRAG_SPEED 0.01f

static constexpr float DegreesDragSpeed = Math::RadToDeg(DRAG_SPEED);


void UIDebug::Init(Window* window)
{
    isInitted = true;

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
static ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;

void UIDebug::TestDraw()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
#ifdef EDITOR
    {
        {
            ImVec2 wSize{
                (float)gRenderSys->GetRenderer()->GetWindow()->GetWidth(),
                (float)gRenderSys->GetRenderer()->GetWindow()->GetHeigth()
            };
            ImGuiViewport* imViewPort = ImGui::GetMainViewport();
            imViewPort->Size = wSize;
            imViewPort->WorkSize = wSize;
            ImGui::DockSpaceOverViewport(0, imViewPort);
        }

        // Outliner
        {
            if (!ImGuizmo::IsOver()) {
                outliner.Update();
            }
            outliner.Draw();
        }

        // File system
        {
            ImGui::Begin("File system");
            // ToDo: change position of simulation buttoin
            if (ImGui::Button(bName.c_str()))
            {
                start_simulation = !start_simulation;
                bName = start_simulation ? "Pause" : "Start";
                if (start_simulation) {
                    onSimulationStart.Broadcast();
                } else {
                    onSimulationEnd.Broadcast();
                }
            }
            ImGui::End();
        }
        
        // Script debugger
        {
            ImGui::Begin("Debugger");
            // ToDo: change position of simulation buttoin
            if (ImGui::Button("Debug menu"))
            {
               gScriptSys->InvokeDebuggerMenu();
            }
            ImGui::End();
        }

        // Viewport in window
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Scene Test", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            UpdateViewportPos();
            if (HandleViewportResize()) {
                ImGui::Image((ImTextureID)(intptr_t)gRenderSys->GetRenderer()->GetColorPassSrt(), ImGui::GetWindowSize());

                isSceneFocused = ImGui::IsWindowFocused();
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
                ImGui::Begin("Inspector");

                ImGui::SeparatorText("Transform Manualy");

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

                if (ImGui::Button("Stop"))
                {
                    outliner.gizmo_focused = false;
                }

                ImGui::SameLine();

                ImGui::RadioButton("World", &gizmoSpace, (int)GizmoSpace::World);
                ImGui::SameLine();
                ImGui::RadioButton("Local", &gizmoSpace, (int)GizmoSpace::Local);

                ImGui::SeparatorText("");

                // Position
                const Math::Vector3 pos = trs.GetLocalPosition();
                Math::Vector3 newPos = pos;
                DrawVec3Control("Position", newPos, DRAG_SPEED);
                if (Math::Vector3::DistanceSquared(newPos, pos) > Math::Epsilon) {
                    trs.SetLocalPosition(newPos);
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
                }

                // Scale
                const Math::Vector3 scale = trs.GetLocalScale();
                Math::Vector3 newScale = scale;
                DrawVec3Control("Scale", newScale, DRAG_SPEED, 0.f, 100.f);
                if (Math::Vector3::DistanceSquared(newScale, scale) > Math::Epsilon) {
                    trs.SetLocalScale(newScale);
                }

                ImGui::End();
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
            }
        }
    }
}
#endif // EDITOR