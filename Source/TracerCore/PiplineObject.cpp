#include "PiplineObject.hpp"
#include "TracerIO.hpp"
#include "Model.hpp"

#include <iostream>
#include <assert.h>

namespace TraceCore
{
    PipelineObject::PipelineObject(VulkanDevice& device, const PipelineConfiguration& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath) :
        _device(device)
    {
        CreateGraphicsPipeline(config, vertexShaderPath, fragmentShaderPath);
    }

    PipelineObject::~PipelineObject()
    {
        vkDestroyShaderModule(_device.GetVkDevice(), _vertexShaderModule, nullptr);
        vkDestroyShaderModule(_device.GetVkDevice(), _fragmentShaderModule, nullptr);
        vkDestroyPipeline(_device.GetVkDevice(), _graphicsPipline, nullptr);
    }

    void PipelineObject::Bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipline);
    }

    PipelineConfiguration PipelineObject::GetDefaultConfiguration(uint32_t width, uint32_t height)
    {
        PipelineConfiguration config{};

        config.InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        config.InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        config.InputAssembly.primitiveRestartEnable = VK_FALSE;

        config.Viewport.x = 0.0f;
        config.Viewport.y = 0.0f;
        config.Viewport.width = static_cast<float>(width);
        config.Viewport.height = static_cast<float>(height);
        config.Viewport.minDepth = 0.0f;
        config.Viewport.maxDepth = 1.0f;
        
        config.Scissor.offset = {0, 0};
        config.Scissor.extent = {width, height};
        
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

        return config;
    }

    void PipelineObject::CreateGraphicsPipeline(const PipelineConfiguration &config, const std::string &vertexShaderPath, const std::string &fragmentShaderPath)
    {
        assert(config.PipelineLayout != VK_NULL_HANDLE && 
            "Cannot create graphics pipeline! No pipeline layout provided in PipelineConfiguration");

        assert(config.RenderPass != VK_NULL_HANDLE && 
            "Cannot create graphics pipeline! No render pass provided in PipelineConfiguration");

        auto vertexShaderCode = TracerUtils::IOHelpers::ReadFile(vertexShaderPath);
        auto fragmentShaderCode = TracerUtils::IOHelpers::ReadFile(fragmentShaderPath);

        CreateShaderModule(*(vertexShaderCode.get()), &_vertexShaderModule);
        CreateShaderModule(*(fragmentShaderCode.get()), &_fragmentShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = _vertexShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = _fragmentShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        auto bindingDescription = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexAttributeDescriptionCount = 1;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInput.pVertexBindingDescriptions = bindingDescription.data();

        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = &config.Viewport;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = &config.Scissor;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &config.InputAssembly;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pRasterizationState = &config.Rasterization;
        pipelineInfo.pMultisampleState = &config.Multisample;
        pipelineInfo.pColorBlendState = &config.ColorBlend;
        pipelineInfo.pDepthStencilState = &config.DepthStencil;
        pipelineInfo.pDynamicState = nullptr;

        pipelineInfo.layout = config.PipelineLayout;
        pipelineInfo.renderPass = config.RenderPass;
        pipelineInfo.subpass = config.Subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if(vkCreateGraphicsPipelines(_device.GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipline) != VK_SUCCESS){
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

    void PipelineObject::CreateShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if(vkCreateShaderModule(_device.GetVkDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create shader module");
        }
    }
}