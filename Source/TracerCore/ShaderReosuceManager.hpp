#pragma once

#include <array>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include "VulkanDevice.hpp"
#include "Texture2D.hpp"

namespace TracerCore
{
    class ShaderReosuceManager
    {
    public:
        ShaderReosuceManager(VulkanDevice& device, int maxImagesInFlight);
        ~ShaderReosuceManager();

        ShaderReosuceManager(const ShaderReosuceManager&) = delete;
        ShaderReosuceManager operator=(const ShaderReosuceManager&) = delete;

        inline VkDescriptorSetLayout* GetDescriptorSetLayout() { return &_descriptorSetLayout; }

        void CreateDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout);
        void CreateDescriptorSets(VkDescriptorSetLayout& descriptorSetLayout);

        void UploadBuffer(VkBuffer buffer, VkDeviceSize stride);
        void UploadTexture(const Texture2D* texture);

        void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int imageIndex);

    private:
        void Init();

        void CreateDescriptorPool();

        VulkanDevice& _device;
        const int _maxImagesInFlight;
        
        VkDescriptorSetLayout _descriptorSetLayout;
        VkDescriptorPool _descriptorPool;
        std::vector<VkDescriptorSet> _descriptorSets;
        
    };
} // namespace TracerCore
