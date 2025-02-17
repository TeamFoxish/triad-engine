#include "Window.h"

#ifdef _WIN32
#include <windows.h>
#include <WinUser.h>
#include <wrl.h>

#include <iostream>
#include "wnd.h"
#include "input/InputDevice.h"

#include "backends/imgui_impl_win32.h"
#include "editor/ui_debug/UIDebug.h"
#include "render/RenderSystem.h"
#include "render/Renderer.h"

#ifdef EDITOR
#include "editor/runtime/EditorRuntime.h"
#endif

class wndWindow : public Window {
public:
	bool Create(const std::string& title, int width, int height) override;
	void Destroy() override;

	// deprecated
	void SetWindowDimensions(int _width, int _height) 
	{ 
		width = _width; 
		height = _height;
	}

	void SetWindowClientDimension(int _width, int _height) 
	{
		clientWidth = _width;
		clientHeight = _height;
	}

	void SetWindowBorderedDimension(int _width, int _height)
	{
		borderedWidth = _width;
		borderedHeight = _height;
	}

	HWND hWnd;
};


Window* osCreateWindow(const std::string& title, int width, int height) 
{
	auto window = new wndWindow();
	if (!window->Create(title, width, height)) {
		return nullptr;
	}
	return window;
}


void osDestroyWindow(Window* window)
{
	window->Destroy();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, umessage, wparam, lparam))
		return true;

	bool ignoreMouse = false;
	bool ignoreKeyboard = false;

#ifdef EDITOR
	using InputTarget = EditorController::InputContextBase::InputTarget;
	InputTarget edtInpTarget =
		static_cast<EditorRuntime*>(gEngineRuntime)->GetController().GetInputTarget();
	ignoreMouse = (edtInpTarget == InputTarget::Editor);
	ignoreKeyboard = false; // TEMP
#endif

	switch (umessage) {
	case WM_SIZE:
	{
		// TODO: migrate to WM_ENTERSIZEMOVE WM_EXITSIZEMOVE events
		UINT width = LOWORD(lparam);
		UINT height = HIWORD(lparam);
		if (!gRenderSys || gRenderSys->GetRenderer() == nullptr) {
			return true;
		}
		wndWindow* window = static_cast<wndWindow*>(gRenderSys->GetRenderer()->GetWindow());
		window->SetWindowDimensions((int)width, (int)height);

		RECT wndRect;
		if (GetWindowRect(window->hWnd, &wndRect)) {
			const int borderedWidth = wndRect.right - wndRect.left;
			const int borderedHeight = wndRect.bottom - wndRect.top;
			window->SetWindowBorderedDimension(borderedWidth, borderedHeight);
		}

		RECT clientRect;
		if (GetClientRect(window->hWnd, &clientRect)) {
			const int clientWidth = clientRect.right;
			const int clientHeight = clientRect.bottom;
			window->SetWindowClientDimension(clientWidth, clientHeight);
		}

		window->windowResized.Broadcast((int)width, (int)height);

		return true;
	}
	case WM_INPUT:
	{
		if (globalInputDevice) {

			UINT dwSize = 0;
			GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
			LPBYTE lpb = new BYTE[dwSize];
			if (lpb == nullptr) {
				return 0;
			}

			if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
				OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

			RAWINPUT* vertFloats = reinterpret_cast<RAWINPUT*>(lpb);

			if (vertFloats->header.dwType == RIM_TYPEKEYBOARD && !ignoreKeyboard)
			{
				//printf(" Kbd: make=%04i Flags:%04i Reserved:%04i ExtraInformation:%08i, msg=%04i VK=%i \n",
				//	raw->data.keyboard.MakeCode,
				//	raw->data.keyboard.Flags,
				//	raw->data.keyboard.Reserved,
				//	raw->data.keyboard.ExtraInformation,
				//	raw->data.keyboard.Message,
				//	raw->data.keyboard.VKey);

				globalInputDevice->OnKeyDown({
					vertFloats->data.keyboard.MakeCode,
					vertFloats->data.keyboard.Flags,
					vertFloats->data.keyboard.VKey,
					vertFloats->data.keyboard.Message
					});
			}
			else if (vertFloats->header.dwType == RIM_TYPEMOUSE && !ignoreMouse)
			{
				//printf(" Mouse: X=%04d Y:%04d \n", raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				globalInputDevice->OnMouseMove({
					vertFloats->data.mouse.usFlags,
					vertFloats->data.mouse.usButtonFlags,
					static_cast<int>(vertFloats->data.mouse.ulExtraInformation),
					static_cast<int>(vertFloats->data.mouse.ulRawButtons),
					static_cast<short>(vertFloats->data.mouse.usButtonData),
					vertFloats->data.mouse.lLastX,
					vertFloats->data.mouse.lLastY
					});
			}

			delete[] lpb;
			return DefWindowProc(hwnd, umessage, wparam, lparam);
		}
	}
	case WM_KEYDOWN: {
		if (static_cast<unsigned int>(wparam) == 27) PostQuitMessage(0);
		return 0;
	}
	default: {
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
	}
}


bool wndWindow::Create(const std::string& title, int _width, int _height)
{
	width = _width;
	height = _height;

	std::wstring appName(title.begin(), title.end());
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	WNDCLASSEX wc;

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = appName.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;

	auto posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	auto posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	hWnd = CreateWindowEx(WS_EX_APPWINDOW, appName.c_str(), appName.c_str(),
		dwStyle,
		posX, posY,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, nullptr, hInstance, nullptr);
	if (hWnd == nullptr) {
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	ShowCursor(true);

	RECT wndRect;
	if (GetWindowRect(hWnd, &wndRect)) {
		borderedWidth = wndRect.right - wndRect.left;
		borderedHeight = wndRect.bottom - wndRect.top;
	}

	RECT clientRect;
	if (GetClientRect(hWnd, &clientRect)) {
		clientWidth = clientRect.right;
		clientHeight = clientRect.bottom;
	}

	return true;
}

void wndWindow::Destroy()
{
	DestroyWindow(hWnd);
}

HWND wndGetHWND(Window* window)
{
	wndWindow* wndW = static_cast<wndWindow*>(window);
	return wndW->hWnd;
}
#endif _WIN32
