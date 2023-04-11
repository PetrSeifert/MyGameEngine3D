#include <my_window.hpp>

//std
#include <stdexcept>

MyWindow::MyWindow(int w, int h, std::string title) : width(w), height(h), windowTitle(title) 
{
	initWindow();
}

MyWindow::~MyWindow() 
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void MyWindow::initWindow() 
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void MyWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) 
{
	if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) 
	{
		throw std::runtime_error("failde to create window surface");
	}
}

void MyWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) 
{
	auto myWindow = reinterpret_cast<MyWindow*>(glfwGetWindowUserPointer(window));
	myWindow->framebufferResized = true;
	myWindow->width = width;
	myWindow->height = height;
}