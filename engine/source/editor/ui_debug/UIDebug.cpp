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

#include "render/RenderSystem.h"
#include "render/Renderer.h"
#include "os/wnd.h"
#include "os/Window.h"


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
}

void UIDebug::StartNewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

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
			ImGui::Begin("Outliner");
			ImGui::End();
		}

		// File system
		{
			ImGui::Begin("File system");
			ImGui::End();
		}

		//// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		//{
		//	static float f = 0.0f;
		//	static int counter = 0;
		//
		//	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
		//	
		//	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		//
		//	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//
		//	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		//		counter++;
		//	ImGui::SameLine();
		//	ImGui::Text("counter = %d", counter);
		//
		//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		//	ImGui::End();
		//}

		// Viewport in window
		{
			ImGui::Begin("Scene Test", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::Image((ImTextureID)(intptr_t)gRenderSys->GetRenderer()->GetColorPassSrt(), ImGui::GetWindowSize());
			isSceneFocused = ImGui::IsWindowFocused();
			ImGui::End();
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
