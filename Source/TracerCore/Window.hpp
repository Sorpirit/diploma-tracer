#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace TraceCore
{

    class Window
    {
    public:
        Window(int width, int height, std::string windowName);
        ~Window();

        Window(const Window&) = delete;
        Window &operator=(const Window&) = delete;

        inline bool ShouldClose() { return !glfwWindowShouldClose(_window); };
        inline VkExtent2D GetExtent() { return {static_cast<uint32_t>(_width), static_cast<uint32_t>(_height)}; };
        inline bool FramebufferResized() { return _framebufferResized; };
        inline void ResetFramebufferResizedFlag() { _framebufferResized = false; };
        
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    private:
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
        void InitWindow();

        int _width;
        int _height;
        bool _framebufferResized = false;

        std::string _windowName;
        GLFWwindow* _window;
    };

}