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
        VkPipeline computePipeline
    ) : _device(device), _bindPoint(bindPoint), _setLayout(setLayout), _descriptorPool(descriptorPool), _descriptorSets(descriptorSets), _pipelineLayout(pipelineLayout), _pipeline(computePipeline)
    {
    }

    PipelineObject::~PipelineObject()
    {
        _descriptorSets.clear();
        vkDestroyDescriptorPool(_device.GetVkDevice(), _descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(_device.GetVkDevice(), _setLayout, nullptr);
        vkDestroyPipelineLayout(_device.GetVkDevice(), _pipelineLayout, nullptr);
        vkDestroyPipeline(_device.GetVkDevice(), _pipeline, nullptr);
    }
    
    void PipelineObject::Bind(VkCommandBuffer commandBuffer, int imageIndex)
    {
        vkCmdBindPipeline(commandBuffer, _bindPoint, _pipeline);
        vkCmdBindDescriptorSets(commandBuffer, _bindPoint, _pipelineLayout, 0, 1, &_descriptorSets[imageIndex], 0, nullptr);
    }
} // namespace TracerCore
