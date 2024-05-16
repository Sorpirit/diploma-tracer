#include "VulkanBuffer.hpp"
#include "Texture2D.hpp"

#include <stdexcept>

namespace TracerCore {
namespace Resources
{
 
    std::unique_ptr<VulkanBuffer> VulkanBuffer::CreateBuffer(VulkanDevice &device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer buffer;
        VkDeviceMemory bufferMemory;

        if (vkCreateBuffer(device.GetVkDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device.GetVkDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device.FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device.GetVkDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(device.GetVkDevice(), buffer, bufferMemory, 0);
        return std::make_unique<VulkanBuffer>(device, buffer, bufferMemory, size, usage, properties);
    }

    VulkanBuffer::VulkanBuffer(VulkanDevice& device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : 
        VulkanResource(device, memory), _buffer(buffer), _size(size), _usage(usage), _properties(properties)
    {
    }

    VulkanBuffer::~VulkanBuffer()
    {
        vkDestroyBuffer(_device.GetVkDevice(), _buffer, nullptr);
        vkFreeMemory(_device.GetVkDevice(), _memory, nullptr);
    }

    void VulkanBuffer::CopyToImage(Texture2D *image)
    {
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
        region.imageExtent = {image->GetWidth(), image->GetHeight(), 1};

        vkCmdCopyBufferToImage(commandBuffer, _buffer, image->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        _device.EndSingleTimeCommands(commandBuffer);
    }

    void VulkanBuffer::CopyToBuffer(VulkanBuffer *dstBuffer)
    {
        VkCommandBuffer commandBuffer = _device.BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = _size;
        vkCmdCopyBuffer(commandBuffer, GetBuffer(), dstBuffer->GetBuffer(), 1, &copyRegion);

        _device.EndSingleTimeCommands(commandBuffer);
    }
}}
