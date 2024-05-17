#pragma once

#include "Window.hpp"
#include "Utils/DebugLayerMessenger.hpp"

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace TracerCore {

    /// @brief Contains the details of the swap chain.
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /// @brief Contains the indices of the queue families required for rendering.
    struct QueueFamilyIndices {

        uint32_t GraphicsFamily;
        uint32_t PresentFamily;
        uint32_t ComputeFamily;

        bool HasGraphicsFamily;
        bool HasPresentFamily;
        bool HasComputeFamily;

        /// @brief Returns true if the required queue families are supported.
        /// @return True if the required queue families are supported.
        inline bool IsComplete() { return HasGraphicsFamily && HasPresentFamily && HasComputeFamily; }

    };

    class VulkanDevice {
        public:
        VulkanDevice(Window &window);
        ~VulkanDevice();

        // Not copyable or movable
        VulkanDevice(const VulkanDevice &) = delete;
        VulkanDevice operator=(const VulkanDevice &) = delete;
        VulkanDevice(VulkanDevice &&) = delete;
        VulkanDevice &operator=(VulkanDevice &&) = delete;

        /// @brief Getter for Vulkan instance.
        inline VkInstance GetVkInstance() { return _instance; }

        /// @brief Getter for Vulkan logical device.
        inline VkDevice GetVkDevice() { return _device; }

        /// @brief Getter for Vulkan physical device.
        inline VkPhysicalDevice GetPhysicalDevice() { return _physicalDevice; }

        /// @brief Returns window surface for rendering.
        inline VkSurfaceKHR GetSurface() { return _surface; }

        /// @brief Retruns pointer to Graphics Queue.
        inline VkQueue GetGraphicsQueue() { return _graphicsQueue; }

        /// @brief Returns pointer to Present Queue.
        inline VkQueue GetPresentQueue() { return _presentQueue; }

        /// @brief Get the swap chain support details for the currently selected physical device. Must be called after the physical device is selected.
        /// @return SwapChainSupportDetails struct with the swap chain support details.
        inline SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(_physicalDevice); }

        VkCommandPool getCommandPool() { return commandPool; }

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        /// @brief Find the queue families supported by currently selected physical device. Must be called after the physical device is selected.
        inline QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(_physicalDevice); }

        VkFormat findSupportedFormat(
            const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        
        VkPhysicalDeviceProperties Properties;

        void initRayTracing(void* ptr);

        private:

#ifdef NDEBUG
        const bool ENABLE_VALIDATION_LAYERS = false;
#else
        const bool ENABLE_VALIDATION_LAYERS = true;
#endif
        const std::vector<const char *> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> DEVICE_EXTENSIONS = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            //ray tracing extensions
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
        };
        
        /// @brief Creates a Vulkan instance. Main entry point for the Vulkan API.
        void CreateInstance();

        /// @brief Create a debug messenger to log validation layer messages.
        void SetupDebugMessenger();

        /// @brief Create a window surface to render to using GLFW.
        inline void CreateSurface();

        /// @brief Pick graphics card to use for rendering based on reqiered features.
        void PickPhysicalDevice();

        /// @brief Create a logical device to interface with the physical device. 
        /// Multiple logical devices can be created for a single physical device. 
        /// Logical devices are used to interface with the physical device.
        void CreateLogicalDevice();

        /// @brief Creates a command pool on grahpics family queue to allocate command buffers from. 
        void CreateGraphicsCommandPool();

        /// @brief Check if the physical graphics card supports the required features.
        bool IsDeviceSuitable(VkPhysicalDevice device);

        /// @brief Returns the queue families supported by the physical device.
        /// @param device Graphics card to check for queue families.
        /// @return QueueFamilyIndices struct with the supported queue families.
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

        /// @brief Get the required extensions for the Vulkan Instance.
        std::vector<const char *> GetRequiredExtensions();
        
        /// @brief Check whether the required validation layers are supported by the Vulkan Instance driver.
        /// @return True if the required validation layers are supported.
        bool CheckValidationLayerSupport();

        /// @brief Check whether the required extensions are suported by Vulkan Instance driver.
        /// @param requiredExtensions List of required extensions.
        /// @param log Weather to log the availabe and reqierd extensions.
        void HasRequierdInstanceExtensions(const std::vector<const char *>& requiredExtensions, bool log);

        /// @brief Check if the physical device supports the required extensions(DEVICE_EXTENSIONS).
        /// @param device Target physical device
        /// @return Returns true if the required extensions are supported.
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

        /// @brief Query the swap chain support details for the physical device.
        /// @param device Currently selected physical device.
        /// @return SwapChainSupportDetails struct with the swap chain support details.
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

        VkInstance _instance;
        VkDebugUtilsMessengerEXT _debugMessenger;
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _device;

        VkQueue _graphicsQueue;
        VkQueue _presentQueue;
        VkQueue _computeQueue;

        Window &window;
        VkCommandPool commandPool;

        
        VkSurfaceKHR _surface;
        

        std::unique_ptr<Utils::DebugLayerMessenger> _debugLayerMessenger;
    };
}