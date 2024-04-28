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
        
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    private:
        void InitWindow();

        const int _width;
        const int _height;

        std::string _windowName;
        GLFWwindow* _window;
    };

}