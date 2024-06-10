#pragma once

#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace TracerCore::UI
{
    class WindowsFileDialog
    {
    public:
        WindowsFileDialog(GLFWwindow* window);
        ~WindowsFileDialog();

        std::string OpenFile(const char* fileFilter);
        std::string SaveFile(const char* fileFilter);
    private:
        GLFWwindow* _window;
    };
}