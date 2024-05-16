
#include "Texture2D.hpp"

#include "TracerIO.hpp"
#include "VulkanBuffer.hpp"

#include <assert.h>

namespace TracerCore {
namespace Resources
{

    void Texture2D::CreateImageWithInfo(
        VulkanDevice &device,
        const VkImageCreateInfo &imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory) 
    {
        if (vkCreateImage(device.GetVkDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device.GetVkDevice(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device.FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device.GetVkDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        if (vkBindImageMemory(device.GetVkDevice(), image, imageMemory, 0) != VK_SUCCESS) {
            throw std::runtime_error("failed to bind image memory!");
        }
    }

    std::unique_ptr<Texture2D> Texture2D::LoadFileTexture(const std::string &filePath, VulkanDevice &device)
    {
        int texWidth, texHeight, texChannels;
        auto imageData = TracerUtils::IOHelpers::LoadImage(filePath, &texWidth, &texHeight, &texChannels, true);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        auto stagingBuffer = VulkanBuffer::CreateBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        void* data;
        stagingBuffer->MapMemory(imageSize, 0, &data);
            memcpy(data, imageData, static_cast<size_t>(imageSize));
        stagingBuffer->UnmapMemory();

        TracerUtils::IOHelpers::FreeImage(imageData);

        auto textureImage = CreateTexture2D(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, true, device);
        textureImage->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        stagingBuffer->CopyToImage(textureImage.get());
        textureImage->TransitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        return std::move(textureImage);
    }

    std::unique_ptr<Texture2D> Texture2D::CreateTexture2D(
            uint32_t texWidth, 
            uint32_t texHeight, 
            VkFormat format,
            VkMemoryPropertyFlags properties,
            VkImageUsageFlags usage,
            bool optimalTiling,
            VulkanDevice& device
    )
    { 
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = texWidth;
        imageInfo.extent.height = texHeight;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = optimalTiling ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;

        CreateImageWithInfo(device, imageInfo, properties, textureImage, textureImageMemory);
        CreateImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView, device);

        VkSampler textureSampler;
        CreateSampler(textureSampler, device);
        return std::make_unique<Texture2D>(texWidth, texHeight, format, VK_IMAGE_LAYOUT_UNDEFINED, textureImage, textureImageMemory, textureImageView, textureSampler, device);
    }

    Texture2D::Texture2D(uint32_t width, uint32_t height, VkFormat format, VkImageLayout layout, VkImage image, VkDeviceMemory imageMemory, VkImageView imageView, VkSampler sampler, VulkanDevice& device)
        : VulkanResource(device, imageMemory), _width(width), _height(height), _format(format), _currentLayout(layout), _image(image), _imageView(imageView), _sampler(sampler)
    { 
    }

    Texture2D::~Texture2D()
    {
        vkDestroySampler(_device.GetVkDevice(), _sampler, nullptr);
        vkDestroyImageView(_device.GetVkDevice(), _imageView, nullptr);
        vkDestroyImage(_device.GetVkDevice(), _image, nullptr);
        vkFreeMemory(_device.GetVkDevice(), _memory, nullptr);
    }

    void Texture2D::TransitionImageLayout(VkImageLayout newLayout)
    {
        assert(_currentLayout != newLayout);
        assert(newLayout != VK_IMAGE_LAYOUT_UNDEFINED);

        VkCommandBuffer commandBuffer = _device.BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = _currentLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = _image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        GetImageLayoutMask(_currentLayout, barrier.srcAccessMask, sourceStage);
        GetImageLayoutMask(newLayout, barrier.dstAccessMask, destinationStage);
        
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        _device.EndSingleTimeCommands(commandBuffer);
        _currentLayout = newLayout;
    }

    void Texture2D::CopyToBuffer(VulkanBuffer *buffer)
    {
        auto startLayout = _currentLayout;
        if(_currentLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        }

        VkCommandBuffer commandBuffer = _device.BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {_width, _height, 1};

        vkCmdCopyImageToBuffer(commandBuffer, _image, _currentLayout, buffer->GetBuffer(), 1, &region);

        _device.EndSingleTimeCommands(commandBuffer);

        if(startLayout != _currentLayout)
        {
            TransitionImageLayout(startLayout);
        }
    }

    void Texture2D::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView &imageView, VulkanDevice &device)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.GetVkDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void Texture2D::CreateSampler(VkSampler &sampler, VulkanDevice &device)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = device.Properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(device.GetVkDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void Texture2D::GetImageLayoutMask(VkImageLayout layout, VkAccessFlags &accessMask, VkPipelineStageFlags &stageMask)
    {
        switch (layout)
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            accessMask = 0;
            stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            accessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            stageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            accessMask = VK_ACCESS_TRANSFER_READ_BIT;
            stageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            accessMask = VK_ACCESS_SHADER_READ_BIT;
            stageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            accessMask = VK_ACCESS_SHADER_READ_BIT;
            stageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;
        default:
            throw std::invalid_argument("unsupported layout transition!");
            break;
        }
    }

}}