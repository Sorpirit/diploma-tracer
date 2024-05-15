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

    /// @brief Represents standart graphics pipeline object. Sequence of operations that tells the GPU how to render a given scene.
    class GraphicsPipelineObject
    {
    public:
        GraphicsPipelineObject(VulkanDevice& device, const PipelineConfiguration& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
        GraphicsPipelineObject(VulkanDevice& device, const PipelineConfiguration& config, const std::string& shaderName);
        ~GraphicsPipelineObject();

        GraphicsPipelineObject(const GraphicsPipelineObject&) = delete;
        GraphicsPipelineObject operator=(const GraphicsPipelineObject&) = delete;
        
        void Bind(VkCommandBuffer commandBuffer);

        static void GetDefaultConfiguration(PipelineConfiguration& config);

    private:
        void CreateGraphicsPipeline(const PipelineConfiguration& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
        void CreateComputePipeline(const PipelineConfiguration& config, const std::string& computeShaderPath);
        
        /// @brief Creates a shader module from the given code.
        void CreateShaderModule(const std::vector<char>* code, VkShaderModule* shaderModule);
        void CreatePipleineStage(
            const std::vector<char>* code, 
            const VkShaderStageFlagBits stage, 
            VkShaderModule* shaderModule, 
            VkPipelineShaderStageCreateInfo& shaderStage);

        VulkanDevice& _device;
        VkPipeline _graphicsPipline;        
    };
}