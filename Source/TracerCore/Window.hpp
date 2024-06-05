#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace TracerCore
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

        inline void LockCursor() {
            if(_isCursorLocked)
                return;

            _isCursorLocked = true;
            glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
        };
        inline void UnlockCursor() {
            if(!_isCursorLocked)
                return;

            _isCursorLocked = false;
            glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        };

        inline GLFWwindow* GetWindow() { return _window; };
        inline void CloseWindow() { glfwSetWindowShouldClose(_window, GLFW_TRUE); };
        
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    private:
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
        void InitWindow();

        int _width;
        int _height;
        bool _framebufferResized = false;

        bool _isCursorLocked;

        std::string _windowName;
        GLFWwindow* _window;
    };

}