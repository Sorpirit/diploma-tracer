#include "Tracer.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "PiplineObject.hpp"


namespace TraceCore
{
    struct SimplePushConstantData
    {
        glm::mat2 transform{1.0f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    Tracer::Tracer()
    {
        LoadModels();
        CreatePipelineLayout();
        RecreateSwapChain();
        CreateCommandBuffers();
        _raytracer.LoadRaytracer(_device);
    }

    Tracer::~Tracer()
    {
        vkDestroyPipelineLayout(_device.GetVkDevice(), _pipelineLayout, nullptr);
    }

    void Tracer::Run()
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::cout << extensionCount << " extensions supported\n";

        while(_mainWindow.ShouldClose()) {
            glfwPollEvents();
            DrawFrame();
        }

        vkDeviceWaitIdle(_device.GetVkDevice());
    }

    // static void GereneateSerpinskyTrangle(std::vector<Vertex>& vertices, int depth, int offset)
    // {
    //     if(depth == 0) {
    //         return;
    //     }

    //     if(depth == 1) {
    //         Vertex n1 = {(vertices[offset].position + vertices[offset + 1].position) / 2.0f};
    //         Vertex n2 = {(vertices[offset + 1].position + vertices[offset + 2].position) / 2.0f};
    //         Vertex n3 = {(vertices[offset].position + vertices[offset + 2].position) / 2.0f};

    //         vertices.push_back(vertices[offset]);

    //         vertices.push_back(vertices[offset]);
    //         vertices.push_back(vertices[offset + 1]);
    //         vertices.push_back(vertices[offset + 2]);
    //         return;
    //     }
    // }

    void Tracer::LoadModels()
    {
        std::vector<Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        _model = std::make_unique<Model>(_device, vertices);
    }

    void Tracer::CreatePipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional
        if(vkCreatePipelineLayout(_device.GetVkDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

    }
    
    void Tracer::CreatePipeline()
    {
        assert(_swapChain != nullptr && "Unable to create pipeline while swap chain is not created");
        assert(_pipelineLayout != nullptr && "Unable to create pipeline while pipeline layout is not created");

        PipelineConfiguration pipelineConfig{};
        PipelineObject::GetDefaultConfiguration(pipelineConfig);

        pipelineConfig.RenderPass = _swapChain->getRenderPass();
        pipelineConfig.PipelineLayout = _pipelineLayout;

        _pipline = std::make_unique<PipelineObject>(_device, pipelineConfig, "PrecompiledShaders\\Flat.vert.spv", "PrecompiledShaders\\Flat.frag.spv");
    }
    
    void Tracer::CreateCommandBuffers()
    {
        _commandBuffers.resize(_swapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = _device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

        if(vkAllocateCommandBuffers(_device.GetVkDevice(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    
    void Tracer::DrawFrame()
    {
        uint32_t imageIndex;
        auto result = _swapChain->acquireNextImage(&imageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return;
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        RecordCommandBuffer(imageIndex);
        result = _swapChain->submitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _mainWindow.FramebufferResized()) {
            _mainWindow.ResetFramebufferResizedFlag();
            RecreateSwapChain();
            return;
        }

        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }

    void Tracer::RecreateSwapChain()
    {
        auto extent = _mainWindow.GetExtent();
        while(extent.width == 0 || extent.height == 0) {
            extent = _mainWindow.GetExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(_device.GetVkDevice());

        if(_swapChain == nullptr)
        {
            _swapChain = std::make_unique<SwapChain>(_device, extent);
        } else {
            _swapChain = std::make_unique<SwapChain>(_device, extent, std::move(_swapChain));

            if(_swapChain->imageCount() != _commandBuffers.size()) {
                FreeCommandBuffers();
                CreateCommandBuffers();
            }
        }
        
        CreatePipeline();
    }

    void Tracer::RecordCommandBuffer(int index)
    {
        static int frame = 0;
        frame = (frame + 1) % 1000;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(_commandBuffers[index], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _swapChain->getRenderPass();
        renderPassInfo.framebuffer = _swapChain->getFrameBuffer(index);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_commandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_swapChain->width());
        viewport.height = static_cast<float>(_swapChain->height());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, _swapChain->getSwapChainExtent()};
        vkCmdSetViewport(_commandBuffers[index], 0, 1, &viewport);
        vkCmdSetScissor(_commandBuffers[index], 0, 1, &scissor);
        
        _pipline->Bind(_commandBuffers[index]);
        _model->Bind(_commandBuffers[index]);

        for (int i = 0; i < 4; i++)
        {
            SimplePushConstantData push{};
            push.offset = {0.0f, -0.4f + i * 0.25f};
            push.color = {0.0f, 0.0f, 0.2f + i * 0.2f};
            vkCmdPushConstants(_commandBuffers[index], _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            _model->Draw(_commandBuffers[index]);
        }

        vkCmdEndRenderPass(_commandBuffers[index]);

        if(vkEndCommandBuffer(_commandBuffers[index]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
    
    void Tracer::FreeCommandBuffers()
    {
        vkFreeCommandBuffers(_device.GetVkDevice(), _device.getCommandPool(), static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
        _commandBuffers.clear();
    }
}