#include "UIDebug.h"

#include <filesystem>
#include <windows.h>
#include <WinUser.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "imgui.h"
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
            outliner.Update();
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

        // Viewport in window
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Scene Test", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            UpdateViewportPos();
            if (HandleViewportResize()) {
                ImGui::Image((ImTextureID)(intptr_t)gRenderSys->GetRenderer()->GetColorPassSrt(), ImGui::GetWindowSize());

                isSceneFocused = ImGui::IsWindowFocused();
            }

            if (outliner.gizmo_focused)
            {
                DrawGizmo();
            }

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
void UIDebug::DrawGizmo()
{
    auto node = outliner.GetSelectedNode();

    if (node.id_ >= 0 && node != outliner.GetRootNode())
    {
        SceneTree::Entity& entity = gSceneTree->Get(node);
        if (entity.isComposite && entity.transform.id_ >= 0) {

            // ImGui window
            {
                ImGui::Begin("Gizmo options");

                ImGui::SeparatorText("Transforms");

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

                ImGui::SeparatorText("");

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
            Math::Transform& trs = SharedStorage::Instance().transforms.AccessWrite(entity.transform);
            Math::Matrix matr = trs.GetMatrix(); // TODO: check if gizmo works for child entities

            ImGuizmo::Manipulate((float*)viewMatrix.m, (float*)projectionMatrix.m, operation, ImGuizmo::LOCAL, (float*)matr.m);

            trs.SetMatrix(matr);
        }
    }
}
#endif // EDITOR