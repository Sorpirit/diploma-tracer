#include "WindowsFileDialog.hpp"

#include <windows.h>
#include <commdlg.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace TracerCore::UI
{
    
    WindowsFileDialog::WindowsFileDialog(GLFWwindow* window) : 
        _window(window)
    {
    }

    WindowsFileDialog::~WindowsFileDialog()
    {
    }

    std::string WindowsFileDialog::OpenFile(const char* fileFilter)
    {
        OPENFILENAME ofn;
        char szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = glfwGetWin32Window(_window);
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = fileFilter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if(GetOpenFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }

        return std::string();
    }

    std::string WindowsFileDialog::SaveFile(const char* fileFilter)
    {
        OPENFILENAME ofn;
        char szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = glfwGetWin32Window(_window);
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = fileFilter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if(GetSaveFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }

        return std::string();
    }

} // namespace TracerCore::UI