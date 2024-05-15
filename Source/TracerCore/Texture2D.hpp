#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stb_image.h>
#include <vulkan/vulkan.h>

#include "VulkanDevice.hpp"

namespace TracerCore
{
    class Texture2D
    {
    public:
        Texture2D(VkImage image, VkDeviceMemory imageMemory, VkImageView imageView, VkSampler sampler, VulkanDevice& device);
        ~Texture2D();
        Texture2D(const Texture2D&) = delete;
        Texture2D operator=(const Texture2D&) = delete;

        static std::unique_ptr<Texture2D> LoadFileTexture(const std::string& filePath, VulkanDevice& device);
        static std::unique_ptr<Texture2D> CreateRuntimeTexture(
            uint32_t texWidth, 
            uint32_t texHeight, 
            VkFormat format,
            VkMemoryPropertyFlags properties,
            VkImageUsageFlags usage,
            VulkanDevice& device);

        inline VkImage GetImage() const { return _image; }
        inline VkImageView GetImageView() const { return _imageView; }
        inline VkSampler GetSampler() const { return _sampler; }
        inline VkDeviceMemory GetImageMemory() const { return _imageMemory; }

        static void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VulkanDevice& device);
        static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VulkanDevice& device);
        static void CopyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height, VulkanDevice& device);
        static void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView, VulkanDevice& device);

    private:
        
        VkImage _image;
        VkDeviceMemory _imageMemory;
        VkImageView _imageView;
        VkSampler _sampler;
        VulkanDevice& _device;
    };
} // namespace TracerCore