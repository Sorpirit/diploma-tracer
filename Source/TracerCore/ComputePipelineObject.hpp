#pragma once

#include <string>

#include "VulkanDevice.hpp"
#include "GraphicsPipelineObject.hpp"

namespace TracerCore {

    class ComputePipelineObject {
    public:
        ComputePipelineObject(VulkanDevice& device, const PipelineConfiguration& config, const std::string& computeShaderPath);
        ~ComputePipelineObject();

        ComputePipelineObject(const ComputePipelineObject&) = delete;
        ComputePipelineObject operator=(const ComputePipelineObject&) = delete;

        void Bind(VkCommandBuffer commandBuffer);

        static void GetDefaultConfiguration(PipelineConfiguration& config);

    private:
        void CreateComputePipelineLayout();
        void CreateComputeDescriptorSetLayout();
        void CreateComputeDescriptorSet();
        void CreateComputeShaderModule();
        void CreateComputePipeline();

        VulkanDevice& _device;
        VkPipeline _computePipeline;
    };

} // namespace TracerCore