#pragma once

#include <string>

class Window {
public:
	virtual bool Create(const std::string& title, int width, int height) = 0;
	virtual void Destroy() = 0;

	int GetWidth() const { return width; }
	int GetHeigth() const { return height; }

protected:
	int width;
	int height;
};

Window* osCreateWindow(const std::string& title, int width, int height);
void osDestroyWindow(Window* window);
