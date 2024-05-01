#pragma once

#include <string>
#include <vector>
#include <memory>

#include "VulkanDevice.hpp"

namespace TraceCore
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


    class PipelineObject
    {
    public:
        PipelineObject(VulkanDevice& device, const PipelineConfiguration& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
        ~PipelineObject();

        PipelineObject(const PipelineObject&) = delete;
        PipelineObject operator=(const PipelineObject&) = delete;
        
        void Bind(VkCommandBuffer commandBuffer);

        static void GetDefaultConfiguration(PipelineConfiguration& config);

    private:
        void CreateGraphicsPipeline(const PipelineConfiguration& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

        void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        VulkanDevice& _device;
        VkPipeline _graphicsPipline;
        VkShaderModule _vertexShaderModule;
        VkShaderModule _fragmentShaderModule;
        
    };
}