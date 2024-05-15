#pragma once

#include "VulkanDevice.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace TracerCore
{

    class SwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(VulkanDevice &deviceRef, VkExtent2D windowExtent);
        SwapChain(VulkanDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
        ~SwapChain();

        SwapChain(const SwapChain &) = delete;
        SwapChain operator=(const SwapChain &) = delete;

        inline VkFramebuffer GetFrameBuffer(int index) { return _swapChainFramebuffers[index]; }
        inline VkRenderPass GetGraphicsRenderPass() { return _graphicsRenderPass; }
        inline VkImageView GetImageView(int index) { return _swapChainImageViews[index]; }
        /// @brief Returns the number of images in the swap chain.
        inline size_t GetImageCount() { return _swapChainImages.size(); }
        inline VkFormat GetImageFormat() { return _swapChainImageFormat; }
        inline VkExtent2D GetExtent() { return _swapChainExtent; }
        inline uint32_t GetWidth() { return _swapChainExtent.width; }
        inline uint32_t GetHeight() { return _swapChainExtent.height; }

        inline float ExtentAspectRatio() { return static_cast<float>(_swapChainExtent.width) / static_cast<float>(_swapChainExtent.height); }
        VkFormat FindDepthFormat();

        VkResult AcquireNextImage(uint32_t *imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    private:
        void Init();
        
        /// @brief Creates the swap chain on queue families that was queried from the physical device. Retrives VkImage handles for the swap chain images.
        void CreateSwapChain();
        
        /// @brief Creates the image views for VkImages retrived after creation of the swap chain.
        void CreateImageViews();

        /// @brief Creates Render pass for the swap chain images.
        void CreateRenderPass();
        
        void createDepthResources();

        /// @brief Creates the framebuffers for the created render pass.
        void CreateFramebuffers();

        /// @brief Creates the semaphores and fences for the swap chain.
        void CreateSyncObjects();

        /// @brief Chooses the best surface format for the swap chain.
        /// @param availableFormats The available formats.
        /// @return The best surface format.
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        /// @brief Chooses the prefered present mode for the swap chain(aka V-Sync, Unlitmited, ect.).
        /// @param availablePresentModes Available present modes.
        /// @return Returns the prefered present mode.
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        /// @brief Chooses the swap extent base on the window extent and the surface capabilities.
        /// @param capabilities The surface capabilities.
        /// @return The swap extent.
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkFormat _swapChainImageFormat;
        VkExtent2D _swapChainExtent;

        std::vector<VkFramebuffer> _swapChainFramebuffers;
        VkRenderPass _graphicsRenderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> _swapChainImages;
        std::vector<VkImageView> _swapChainImageViews;

        VulkanDevice& _device;
        VkExtent2D _windowExtent;

        VkSwapchainKHR _swapChain;
        std::shared_ptr<SwapChain> _oldSwapChain;

        std::vector<VkSemaphore> _imageAvailableSemaphores;
        std::vector<VkSemaphore> _renderFinishedSemaphores;
        std::vector<VkFence> _inFlightFences;
        std::vector<VkFence> _imagesInFlight;
        size_t _currentFrame = 0;
    };

}