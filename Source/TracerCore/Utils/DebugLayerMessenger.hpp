#pragma once

#include <vulkan/vulkan.h>

namespace TracerCore {
namespace Utils
{
    
    class DebugLayerMessenger
    {
    public:

        static void GetDefaultCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        /// @brief Construct a new Debug Layer Messenger object
        /// @param createInfo Information about the debug messenger
        /// @param instance Pointer to the Vulkan instance
        DebugLayerMessenger(const VkDebugUtilsMessengerCreateInfoEXT* createInfo, VkInstance instance);
        ~DebugLayerMessenger();

        DebugLayerMessenger(const DebugLayerMessenger &) = delete;
        DebugLayerMessenger operator=(const DebugLayerMessenger &) = delete;
        DebugLayerMessenger(DebugLayerMessenger &&) = delete;
        DebugLayerMessenger &operator=(DebugLayerMessenger &&) = delete;

    private:
        VkInstance _instance;
        VkDebugUtilsMessengerEXT _debugMessenger;

        //function pointers
        PFN_vkCreateDebugUtilsMessengerEXT _vkCreateDebugUtilsMessengerEXT;
        PFN_vkDestroyDebugUtilsMessengerEXT _vkDestroyDebugUtilsMessengerEXT;
    };

} // namespace Utils
} // namespace TracerCore; 
