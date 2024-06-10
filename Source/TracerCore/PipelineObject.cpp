#include "PipelineObject.hpp"

namespace TracerCore
{
    PipelineObject::PipelineObject(
        VulkanDevice &device, 
        VkPipelineBindPoint bindPoint, 
        VkDescriptorSetLayout setLayout, 
        VkDescriptorPool descriptorPool, 
        std::vector<VkDescriptorSet> descriptorSets, 
        VkPipelineLayout pipelineLayout, 
        std::vector<VkPipeline> pipelines
    ) : _device(device), _bindPoint(bindPoint), _setLayout(setLayout), _descriptorPool(descriptorPool), _descriptorSets(descriptorSets), _pipelineLayout(pipelineLayout), _pipelineVariants(pipelines)
    {
    }

    PipelineObject::~PipelineObject()
    {
        _descriptorSets.clear();
        vkDestroyDescriptorPool(_device.GetVkDevice(), _descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(_device.GetVkDevice(), _setLayout, nullptr);
        vkDestroyPipelineLayout(_device.GetVkDevice(), _pipelineLayout, nullptr);
        for (const auto& pipeline : _pipelineVariants)
        {
            vkDestroyPipeline(_device.GetVkDevice(), pipeline, nullptr);
        }
        _pipelineVariants.clear();
    }
    
    void PipelineObject::Bind(VkCommandBuffer commandBuffer, int imageIndex)
    {
        vkCmdBindPipeline(commandBuffer, _bindPoint, _pipelineVariants[_pipelineVariantIndex]);
        vkCmdBindDescriptorSets(commandBuffer, _bindPoint, _pipelineLayout, 0, 1, &_descriptorSets[imageIndex], 0, nullptr);
    }
} // namespace TracerCore
