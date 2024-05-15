#include "Tracer.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "GraphicsPipelineObject.hpp"


namespace TracerCore
{
    struct SimplePushConstantData
    {
        glm::mat2 transform{1.0f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    Tracer::Tracer()
    {
        RecreateSwapChain();
        LoadModels();
        LoadImages();
        _shaderResourceManager = std::make_unique<ShaderReosuceManager>(_device, _swapChain->GetImageCount());
        _shaderResourceManager->UploadTexture(_texture2d.get());
        CreatePipelineLayout();
        CreatePipeline();
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

    void Tracer::LoadModels()
    {
        std::vector<Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        _model = std::make_unique<Model>(_device, vertices);
    }

    void Tracer::LoadImages()
    {
        _texture2d = Texture2D::LoadFileTexture("Textures\\cutecat.jpg", _device);

        const uint32_t texWidth = 512;
        const uint32_t texHeight = 512;

        _computeTexture = Texture2D::CreateRuntimeTexture(texWidth, texHeight, 
            VK_FORMAT_R8G8B8A8_UNORM, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT  | VK_IMAGE_USAGE_SAMPLED_BIT,
            _device
        );

        VkDeviceSize imageSize = texWidth * texHeight * 4;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        _device.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        uint32_t imageData[texWidth * texHeight];

        for (auto i = 0; i < texWidth; i++)
        {
            for (auto j = 0; j < texHeight; j++)
            {
                auto x = i / static_cast<float>(texWidth);
                auto y = j / static_cast<float>(texHeight);
                imageData[i + j * 500] = 0xff000000 | static_cast<uint32_t>(x * 255.0f) << 8 | static_cast<uint32_t>(y * 255.0f);
            }
        }

        void* data;
        vkMapMemory(_device.GetVkDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, imageData, static_cast<size_t>(imageSize));
        vkUnmapMemory(_device.GetVkDevice(), stagingBufferMemory);

        Texture2D::TransitionImageLayout(_computeTexture->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _device);
        Texture2D::CopyBufferToImage(stagingBuffer, _computeTexture->GetImage(), texWidth, texHeight, _device);
        Texture2D::TransitionImageLayout(_computeTexture->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, _device);
    
        vkDestroyBuffer(_device.GetVkDevice(), stagingBuffer, nullptr);
        vkFreeMemory(_device.GetVkDevice(), stagingBufferMemory, nullptr);
    }

    void Tracer::CreatePipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; // Optional
        pipelineLayoutInfo.pSetLayouts = _shaderResourceManager->GetDescriptorSetLayout(); // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
        if(vkCreatePipelineLayout(_device.GetVkDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

    }
    
    void Tracer::CreatePipeline()
    {
        assert(_swapChain != nullptr && "Unable to create pipeline while swap chain is not created");
        assert(_pipelineLayout != nullptr && "Unable to create pipeline while pipeline layout is not created");

        PipelineConfiguration pipelineConfig{};
        GraphicsPipelineObject::GetDefaultConfiguration(pipelineConfig);

        pipelineConfig.RenderPass = _swapChain->GetGraphicsRenderPass();
        pipelineConfig.PipelineLayout = _pipelineLayout;

        //_pipline = std::make_unique<GraphicsPipelineObject>(_device, pipelineConfig, "PrecompiledShaders\\Flat.vert.spv", "PrecompiledShaders\\Flat.frag.spv");
        _pipline = std::make_unique<GraphicsPipelineObject>(_device, pipelineConfig, "PrecompiledShaders\\OnScreen.vert.spv", "PrecompiledShaders\\OnScreen.frag.spv");
    }
    
    void Tracer::CreateCommandBuffers()
    {
        _commandBuffers.resize(_swapChain->GetImageCount());

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
        auto result = _swapChain->AcquireNextImage(&imageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return;
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        RecordCommandBuffer(imageIndex);
        result = _swapChain->SubmitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);

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
        while(extent.width == 0 || extent.height == 0) 
        {
            extent = _mainWindow.GetExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(_device.GetVkDevice());

        if(_swapChain == nullptr)
        {
            _swapChain = std::make_unique<SwapChain>(_device, extent);
        } 
        else 
        {
            _swapChain = std::make_unique<SwapChain>(_device, extent, std::move(_swapChain));

            if(_swapChain->GetImageCount() != _commandBuffers.size()) 
            {
                FreeCommandBuffers();
                CreateCommandBuffers();
            }

            CreatePipeline();
        }
    }

    void Tracer::RecordCommandBuffer(int index)
    {
        vkResetCommandBuffer(_commandBuffers[index], 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if(vkBeginCommandBuffer(_commandBuffers[index], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _swapChain->GetGraphicsRenderPass();
        renderPassInfo.framebuffer = _swapChain->GetFrameBuffer(index);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _swapChain->GetExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_commandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_swapChain->GetWidth());
        viewport.height = static_cast<float>(_swapChain->GetHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, _swapChain->GetExtent()};
        vkCmdSetViewport(_commandBuffers[index], 0, 1, &viewport);
        vkCmdSetScissor(_commandBuffers[index], 0, 1, &scissor);
        
        _pipline->Bind(_commandBuffers[index]);
        _shaderResourceManager->BindDescriptorSet(_commandBuffers[index], _pipelineLayout, index);
        //_model->Bind(_commandBuffers[index]);
        vkCmdDraw(_commandBuffers[index], 6, 1, 0, 0);
        
        // for (int i = 0; i < 4; i++)
        // {
        //     SimplePushConstantData push{};
        //     push.offset = {0.0f, -0.4f + i * 0.25f};
        //     push.color = {0.0f, 0.0f, 0.2f + i * 0.2f};
        //     vkCmdPushConstants(_commandBuffers[index], _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
        //     //_model->Draw(_commandBuffers[index]);
        // }

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