
#include "ComputePipelineObject.hpp"

#include "TracerIO.hpp"

#include <iostream>
#include <assert.h>

namespace TracerCore
{
    ComputePipelineObject::ComputePipelineObject(VulkanDevice &device, const PipelineConfiguration &config, const std::string &computeShaderPath) :
        _device(device)
    {
        assert(config.PipelineLayout != VK_NULL_HANDLE && 
            "Cannot create graphics pipeline! No pipeline layout provided in PipelineConfiguration");

        assert(config.RenderPass != VK_NULL_HANDLE && 
            "Cannot create graphics pipeline! No render pass provided in PipelineConfiguration");

        auto computeShaderCode = TracerUtils::IOHelpers::ReadFile(computeShaderPath);
        VkShaderModule computeShaderModule = {};
        VkPipelineShaderStageCreateInfo computeShaderStage = {};

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = computeShaderCode->size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(computeShaderCode->data());

        if(vkCreateShaderModule(_device.GetVkDevice(), &createInfo, nullptr, &computeShaderModule) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create shader module");
        }

        VkPipelineShaderStageCreateInfo shaderStage;
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStage.module = computeShaderModule;
        shaderStage.pName = "main";
        shaderStage.flags = 0;
        shaderStage.pNext = nullptr;
        shaderStage.pSpecializationInfo = nullptr;
        vkDestroyShaderModule(_device.GetVkDevice(), computeShaderModule, nullptr);
    }

    void ComputePipelineObject::GetDefaultConfiguration(PipelineConfiguration &config)
    {
        config.InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        config.InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        config.InputAssembly.primitiveRestartEnable = VK_FALSE;

        config.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        config.ViewportInfo.viewportCount = 1;
        config.ViewportInfo.scissorCount = 1;
        config.ViewportInfo.pScissors = nullptr; // dynamicly set
        config.ViewportInfo.pViewports = nullptr; // dynamicly set
        
        config.Rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        config.Rasterization.depthClampEnable = VK_FALSE;
        config.Rasterization.rasterizerDiscardEnable = VK_FALSE;
        config.Rasterization.polygonMode = VK_POLYGON_MODE_FILL;
        config.Rasterization.lineWidth = 1.0f;
        config.Rasterization.cullMode = VK_CULL_MODE_NONE;
        config.Rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
        config.Rasterization.depthBiasEnable = VK_FALSE;
        config.Rasterization.depthBiasConstantFactor = 0.0f;  // Optional 
        config.Rasterization.depthBiasClamp = 0.0f;           // Optional
        config.Rasterization.depthBiasSlopeFactor = 0.0f;     // Optional
        
        config.Multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        config.Multisample.sampleShadingEnable = VK_FALSE;
        config.Multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        config.Multisample.minSampleShading = 1.0f;           // Optional
        config.Multisample.pSampleMask = nullptr;             // Optional
        config.Multisample.alphaToCoverageEnable = VK_FALSE;  // Optional
        config.Multisample.alphaToOneEnable = VK_FALSE;       // Optional
        
        config.ColorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        config.ColorBlendAttachment.blendEnable = VK_FALSE;
        config.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        config.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        config.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        config.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        config.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        config.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
        
        config.ColorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        config.ColorBlend.logicOpEnable = VK_FALSE;
        config.ColorBlend.logicOp = VK_LOGIC_OP_COPY;  // Optional
        config.ColorBlend.attachmentCount = 1;
        config.ColorBlend.pAttachments = &config.ColorBlendAttachment;
        config.ColorBlend.blendConstants[0] = 0.0f;  // Optional
        config.ColorBlend.blendConstants[1] = 0.0f;  // Optional
        config.ColorBlend.blendConstants[2] = 0.0f;  // Optional
        config.ColorBlend.blendConstants[3] = 0.0f;  // Optional
        
        config.DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        config.DepthStencil.depthTestEnable = VK_TRUE;
        config.DepthStencil.depthWriteEnable = VK_TRUE;
        config.DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        config.DepthStencil.depthBoundsTestEnable = VK_FALSE;
        config.DepthStencil.minDepthBounds = 0.0f;  // Optional
        config.DepthStencil.maxDepthBounds = 1.0f;  // Optional
        config.DepthStencil.stencilTestEnable = VK_FALSE;
        config.DepthStencil.front = {};  // Optional
        config.DepthStencil.back = {};   // Optional

        config.DynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        config.DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        config.DynamicState.dynamicStateCount = static_cast<uint32_t>(config.DynamicStateEnables.size());
        config.DynamicState.pDynamicStates = config.DynamicStateEnables.data();
        config.DynamicState.flags = 0;
    }

    void ComputePipelineObject::Bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _computePipeline);
    }
} // namespace TracerCore
