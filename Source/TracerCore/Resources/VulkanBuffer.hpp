#pragma once

#include <memory>

#include "VulkanResource.hpp"

namespace TracerCore {
namespace Resources
{
    class Texture2D;

    class VulkanBuffer : public VulkanResource
    {
    public:
        VulkanBuffer(VulkanDevice& device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        ~VulkanBuffer();

        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer operator=(const VulkanBuffer&) = delete;

        void CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size);

        inline VkBuffer GetBuffer() const { return _buffer; }
        inline VkDeviceSize GetSize() const { return _size; }

        void CopyToImage(Texture2D* image);
        void CopyToBuffer(VulkanBuffer* dstBuffer);

        static std::unique_ptr<VulkanBuffer> CreateBuffer(VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    private:
        const VkDeviceSize _size;
        const VkBufferUsageFlags _usage;
        const VkMemoryPropertyFlags _properties;

        VkBuffer _buffer;
    };

}}