#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "VulkanDevice.hpp"

namespace TracerCore
{
    class PipelineObject
    {
    public:
        PipelineObject(
            VulkanDevice& device, 
            VkPipelineBindPoint bindPoint,
            VkDescriptorSetLayout setLayout,
            VkDescriptorPool descriptorPool,
            std::vector<VkDescriptorSet> descriptorSets,
            VkPipelineLayout pipelineLayout,
            std::vector<VkPipeline> pipelines
        );
        ~PipelineObject();

        PipelineObject(const PipelineObject&) = delete;
        PipelineObject operator=(const PipelineObject&) = delete;

        void Bind(VkCommandBuffer commandBuffer, int imageIndex);
        
        inline const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return _descriptorSets; }
        inline void SetPipelineVariantIndex(uint32_t index) { _pipelineVariantIndex = index; }
    private:
        VulkanDevice& _device;

        VkPipelineBindPoint _bindPoint;

        VkDescriptorPool _descriptorPool;
        VkDescriptorSetLayout _setLayout;
        std::vector<VkDescriptorSet> _descriptorSets;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkPipeline> _pipelineVariants;

        uint32_t _pipelineVariantIndex = 0;
    };
}