#pragma once

#include <string>
#include "misc/Delegates.h"

class Window {
public:
	virtual bool Create(const std::string& title, int width, int height) = 0;
	virtual void Destroy() = 0;

	int GetWidth() const { return width; }
	int GetHeigth() const { return height; }

	int GetClientWidth() const { return clientWidth; }
	int GetClientHeight() const { return clientHeight; }

	int GetBorderedWidth() const { return borderedWidth; }
	int GetBorderedHeight() const { return borderedHeight; }

	MulticastDelegate<int, int> windowResized;

protected:
	// deprecated
	int width = 0;
	int height = 0;

	// actually equal to width and height for win32
	int clientWidth = 0;
	int clientHeight = 0;

	int borderedWidth = 0;
	int borderedHeight = 0;
};

Window* osCreateWindow(const std::string& title, int width, int height);
void osDestroyWindow(Window* window);
