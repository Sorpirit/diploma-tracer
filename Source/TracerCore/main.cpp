#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#include "Tracer.hpp"
#include "TracerIO.hpp"

int main() {
    TracerUtils::IOHelpers::SetAssetFolder("..\\Assets");

    TracerCore::Tracer engine{}; 
    
    try
    {
        engine.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}