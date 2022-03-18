#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <string>

class MyWindow {
    public:
        MyWindow(int w, int h, std::string title);
        ~MyWindow();

        MyWindow(const MyWindow&) = delete;
        MyWindow& operator=(const MyWindow&) = delete;
        
        bool shouldClose() { return glfwWindowShouldClose(window); }
        VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
        bool wasWindowResized() { return framebufferResized; }
        void resetWindowResizedFlag() { framebufferResized = false; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
        GLFWwindow* getGLFWWindow() { return window; }

    private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        void initWindow();

        int width;
        int height;
        bool framebufferResized = false;

        std::string windowTitle;
        GLFWwindow* window;
};