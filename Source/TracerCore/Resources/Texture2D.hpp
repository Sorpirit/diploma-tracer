#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stb_image.h>
#include <vulkan/vulkan.h>

#include "VulkanResource.hpp"

namespace TracerCore {
namespace Resources
{
    class VulkanBuffer;

    class Texture2D : public VulkanResource
    {
    public:
        Texture2D(uint32_t width, uint32_t height, VkFormat format, VkImageLayout layout, VkImage image, VkDeviceMemory imageMemory, VkImageView imageView, VkSampler sampler, VulkanDevice& device);
        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D operator=(const Texture2D&) = delete;

        static std::unique_ptr<Texture2D> CreateTexture2D(
            uint32_t texWidth, 
            uint32_t texHeight, 
            VkFormat format,
            VkMemoryPropertyFlags properties,
            VkImageUsageFlags usage,
            bool optimalTiling,
            VulkanDevice& device);
        static std::unique_ptr<Texture2D> LoadFileTexture(const std::string& filePath, VulkanDevice& device);
        static void CreateImageWithInfo(VulkanDevice &device, const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
        
        inline VkImage GetImage() const { return _image; }
        inline VkImageView GetImageView() const { return _imageView; }
        inline VkSampler GetSampler() const { return _sampler; }
        inline VkDeviceMemory GetImageMemory() const { return _memory; }

        inline uint32_t GetWidth() { return _width; }
        inline uint32_t GetHeight() { return _height; }
        
        void TransitionImageLayout(VkImageLayout newLayout);

        void CopyToBuffer(VulkanBuffer* buffer);
    private:
        const uint32_t _width;
        const uint32_t _height;
        const VkFormat _format;

        VkImageLayout _currentLayout;

        VkImage _image;
        VkImageView _imageView;
        VkSampler _sampler;

        static void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView, VulkanDevice& device);
        static void CreateSampler(VkSampler& sampler, VulkanDevice& device);

        void GetImageLayoutMask(VkImageLayout layout, VkAccessFlags& accessMask, VkPipelineStageFlags& stageMask);
    };

}}