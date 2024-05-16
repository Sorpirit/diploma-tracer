#pragma once

#include <string>
#include <vector>
#include <memory>

#include "VulkanDevice.hpp"

namespace TracerCore
{
    struct PipelineConfiguration
    {
        PipelineConfiguration(const PipelineConfiguration&) = delete;
        PipelineConfiguration operator=(const PipelineConfiguration&) = delete;

        VkPipelineViewportStateCreateInfo ViewportInfo;
        VkPipelineInputAssemblyStateCreateInfo InputAssembly;
        VkPipelineRasterizationStateCreateInfo Rasterization;
        VkPipelineMultisampleStateCreateInfo Multisample;
        VkPipelineColorBlendAttachmentState ColorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo ColorBlend;
        VkPipelineDepthStencilStateCreateInfo DepthStencil;
        std::vector<VkDynamicState> DynamicStateEnables;
        VkPipelineDynamicStateCreateInfo DynamicState;
        VkPipelineLayout PipelineLayout = nullptr;
        VkRenderPass RenderPass = nullptr;
        uint32_t Subpass = 0;
    };

    class PipelineManager
    {
    public:
        PipelineManager(VulkanDevice& device);
        ~PipelineManager();

        PipelineManager(const PipelineManager&) = delete;
        PipelineManager operator=(const PipelineManager&) = delete;

        static void GetDefaultConfiguration(PipelineConfiguration& config);

        void CreatePipelineLayout(const VkDescriptorSetLayout& setLayout, VkPipelineLayout* pipelineLayout);

        void CreateGraphicsPipeline(const PipelineConfiguration& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, VkPipeline* pipeline);
        void CreateComputePipeline(const VkPipelineLayout config, const std::string& computeShaderPath, VkPipeline* pipeline);

    private:
        VulkanDevice& _device;

        void CreateShaderModule(const std::vector<char>* code, VkShaderModule* shaderModule);
        void CreatePipleineStage(
            const std::vector<char>* code, 
            const VkShaderStageFlagBits stage, 
            VkShaderModule* shaderModule, 
            VkPipelineShaderStageCreateInfo& shaderStage);
    };
}