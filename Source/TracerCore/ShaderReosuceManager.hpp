#pragma once

#include <array>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include "VulkanDevice.hpp"
#include "Resources/Texture2D.hpp"
#include "Resources/VulkanBuffer.hpp"

namespace TracerCore
{
    class ShaderReosuceManager
    {
    public:
        ShaderReosuceManager(VulkanDevice& device);
        ~ShaderReosuceManager();

        ShaderReosuceManager(const ShaderReosuceManager&) = delete;
        ShaderReosuceManager operator=(const ShaderReosuceManager&) = delete;

        void CreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding* layoutBinding, int bindingCount, VkDescriptorSetLayout& descriptorSetLayout);
        void CreateDescriptorPool(const VkDescriptorPoolSize* poolSizes, int poolSizeCount, int maxSets, VkDescriptorPool& descriptorPool);
        void CreateDescriptorSets(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, int setCount, std::vector<VkDescriptorSet>& descriptorSets);

        void UploadBuffer(
            std::vector<VkDescriptorSet>& descriptorSets, 
            int dtsBinding, 
            VkDescriptorType descriptorType, 
            const Resources::VulkanBuffer* buffer);
        void UploadTexture(
            std::vector<VkDescriptorSet>& descriptorSets, 
            int dtsBinding, 
            VkImageLayout targetLayout,
            VkDescriptorType descriptorType, 
            const Resources::Texture2D* texture);
            
    private:
        void Init();

        VulkanDevice& _device;
    };
} // namespace TracerCore
