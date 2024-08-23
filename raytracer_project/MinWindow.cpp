
#include "MinWindow.h"

#include <iostream>



static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

bool MinWindow::init(unsigned width, unsigned height, const char* title)
{
	// GLFW
	//
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return false;

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window)
	{
		fprintf(stderr, "Failed to create GLFW m_window\n");
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

bool MinWindow::windowShouldClose() const
{
	return (bool)glfwWindowShouldClose(window);
}

void MinWindow::pollEvents() const
{
	glfwPollEvents();
}

void MinWindow::processInput() const
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void MinWindow::putPixel(int x, int y, const Color& color) const
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y, 1, 1); /// position of pixel
	glClearColor(color.x, color.y, color.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

void MinWindow::flush(const Color& color)
{
	glfwGetFramebufferSize(window, &displayW, &displayH);
	glfwSwapBuffers(window);
	glClearColor(color.x, color.y, color.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void MinWindow::shutdown() const
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void MinWindow::swapBuffers()
{
	glfwSwapBuffers(window);
}
