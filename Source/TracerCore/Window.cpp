#include "Window.hpp"

#include <stdexcept>

namespace TracerCore
{

    Window::Window(int width, int height, std::string windowName) :
        _width(width), _height(height), _windowName(windowName)
    {
        InitWindow();
    }

    Window::~Window()
    {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if(glfwCreateWindowSurface(instance, _window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("filed to create window surface");
        }
    }

    void Window::FramebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto tracerWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        tracerWindow->_framebufferResized = true;

        tracerWindow->_width = width;
        tracerWindow->_height = height;
    }

    void Window::InitWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        _window = glfwCreateWindow(_width, _height, _windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
    }

}


