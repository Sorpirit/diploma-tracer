#pragma once

#include "..\VulkanDevice.hpp"

namespace TracerCore {
namespace Resources
{
    class VulkanResource
    {
    public:
        virtual ~VulkanResource() = default;

        VulkanResource(const VulkanResource&) = delete;
        VulkanResource operator=(const VulkanResource&) = delete;

        inline VkResult MapMemory(VkDeviceSize size, VkDeviceSize offset, void** data) const 
        {
            return vkMapMemory(_device.GetVkDevice(), _memory, offset, size, 0, data);
        }

        inline void UnmapMemory() const
        {
            vkUnmapMemory(_device.GetVkDevice(), _memory);
        }

    protected:
        VulkanResource(VulkanDevice& device, VkDeviceMemory memory) 
            : _device(device), _memory(memory) { }

        VulkanDevice& _device;
        VkDeviceMemory _memory;

    };
}}