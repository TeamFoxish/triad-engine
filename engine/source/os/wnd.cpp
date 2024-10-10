#include "Window.h"

#ifdef _WIN32
#include <windows.h>
#include <WinUser.h>
#include <wrl.h>

#include <iostream>
#include "wnd.h"
#include "input/InputDevice.h"


class wndWindow : public Window {
public:
	bool Create(const std::string& title, int width, int height) override;
	void Destroy() override;

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


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage) {
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

			if (vertFloats->header.dwType == RIM_TYPEKEYBOARD)
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
			else if (vertFloats->header.dwType == RIM_TYPEMOUSE)
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
		// If a key is pressed send it to the input object so it can record that state.
		std::cout << "Key: " << static_cast<unsigned int>(wparam) << std::endl;

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

	auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;

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
