#pragma once

#include "Material.h" // for Color
#include <GLFW/glfw3.h>


class MinWindow
{
public:
	bool init(unsigned width, unsigned height, const char* title);
	bool windowShouldClose() const;
	void pollEvents() const;
	void processInput() const;
	void putPixel(int x, int y, const Color& color) const;
	void flush(const Color& color);
	void shutdown() const;
	void swapBuffers();
	GLFWwindow* getWindow() { return window; }
	int getWidth() { return displayW; }
	int getHeight() { return displayH; }

private:
	GLFWwindow* window;
	int displayW;
	int displayH;
};

