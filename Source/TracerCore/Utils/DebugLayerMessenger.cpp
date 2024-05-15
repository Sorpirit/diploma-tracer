#include "DebugLayerMessenger.hpp"

#include <iostream>
#include <stdexcept>

namespace TracerCore {
namespace Utils
{

    // local callback functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugLayerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) 
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    DebugLayerMessenger::DebugLayerMessenger(const VkDebugUtilsMessengerCreateInfoEXT *createInfo, VkInstance instance) : _instance(instance)
    {
        _vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) 
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        _vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) 
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (_vkCreateDebugUtilsMessengerEXT == nullptr || _vkDestroyDebugUtilsMessengerEXT == nullptr) 
        {
            throw std::runtime_error("Failed to load debug messenger function pointers");
        }

        if(_vkCreateDebugUtilsMessengerEXT(instance, createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to set up debug messenger");
        }
    }

    DebugLayerMessenger::~DebugLayerMessenger()
    {
        _vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    }

    void DebugLayerMessenger::GetDefaultCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugLayerCallback;
        createInfo.pUserData = nullptr; // Optional
    }

} // namespace Utils
} // namespace TracerCore;