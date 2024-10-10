#pragma once

struct HWND__;
typedef HWND__* HWND;

class Window;

HWND wndGetHWND(Window* window);
