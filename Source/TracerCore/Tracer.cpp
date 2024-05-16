#include "Tracer.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


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
        CreateOnScreenPipelines();
        CreateComputePipelines();

        CreateCommandBuffers();
        _raytracer.LoadRaytracer(_device);
    }

    Tracer::~Tracer()
    {
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
        _texture2d = Resources::Texture2D::LoadFileTexture("Textures\\cutecat.jpg", _device);

        const uint32_t texWidth = 512;
        const uint32_t texHeight = 512;

        _computeTexture = Resources::Texture2D::CreateTexture2D(texWidth, texHeight, 
            VK_FORMAT_R8G8B8A8_UNORM, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT  | VK_IMAGE_USAGE_SAMPLED_BIT,
            false,
            _device
        );
    }

    void Tracer::CreateOnScreenPipelines()
    {
        assert(_swapChain != nullptr && "Unable to create pipeline while swap chain is not created");

        //Create graphics pipeline;
        auto imageCount = _swapChain->GetImageCount();

        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout setLayout;
        std::vector<VkDescriptorSet> descriptorSets;
        VkPipelineLayout pipelineLayout;
        VkPipeline onScreenPipeline;

        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount}
        };
        VkDescriptorSetLayoutBinding layoutBindings[1];
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[0].pImmutableSamplers = nullptr;

        _shaderResourceManager.CreateDescriptorPool(poolSizes, 1, imageCount, descriptorPool);
        _shaderResourceManager.CreateDescriptorSetLayout(layoutBindings, 1, setLayout);
        _shaderResourceManager.CreateDescriptorSets(descriptorPool, setLayout, imageCount, descriptorSets);

        _pipelineManager.CreatePipelineLayout(setLayout, &pipelineLayout);

        PipelineConfiguration pipelineConfig{};
        PipelineManager::GetDefaultConfiguration(pipelineConfig);
        pipelineConfig.RenderPass = _swapChain->GetGraphicsRenderPass();
        pipelineConfig.PipelineLayout = pipelineLayout;

        _pipelineManager.CreateGraphicsPipeline(pipelineConfig,  "PrecompiledShaders\\OnScreen.vert.spv", "PrecompiledShaders\\OnScreen.frag.spv", &onScreenPipeline);

        _shaderResourceManager.UploadTexture(descriptorSets, 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _computeTexture.get());

        _graphicsPipeline = std::make_unique<PipelineObject>(
            _device,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            setLayout,
            descriptorPool, 
            std::move(descriptorSets),
            pipelineLayout,
            onScreenPipeline
        );
    }

    void Tracer::CreateComputePipelines()
    {
        assert(_swapChain != nullptr && "Unable to create pipeline while swap chain is not created");

        //Create compute pipeline
        auto imageCount = _swapChain->GetImageCount();

        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout setLayout;
        std::vector<VkDescriptorSet> descriptorSets;
        VkPipelineLayout pipelineLayout;
        VkPipeline computePipeline;

        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount}
        };
        VkDescriptorSetLayoutBinding layoutBindings[1];
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[0].pImmutableSamplers = nullptr;

        _shaderResourceManager.CreateDescriptorPool(poolSizes, 1, imageCount, descriptorPool);
        _shaderResourceManager.CreateDescriptorSetLayout(layoutBindings, 1, setLayout);
        _shaderResourceManager.CreateDescriptorSets(descriptorPool, setLayout, imageCount, descriptorSets);

        _pipelineManager.CreatePipelineLayout(setLayout, &pipelineLayout);

        _pipelineManager.CreateComputePipeline(pipelineLayout, "PrecompiledShaders\\Raytracing.comp.spv", &computePipeline);

        _shaderResourceManager.UploadTexture(descriptorSets, 0, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, _computeTexture.get());
    
        _computePipeline = std::make_unique<PipelineObject>(
            _device,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            setLayout,
            descriptorPool, 
            std::move(descriptorSets),
            pipelineLayout,
            computePipeline
        );
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

        

        //Run compute pipeline
        _computeTexture->TransitionImageLayout(VK_IMAGE_LAYOUT_GENERAL);
        auto cmdBuffer = _device.BeginSingleTimeCommands();
        _computePipeline->Bind(cmdBuffer, imageIndex);
        vkCmdDispatch(cmdBuffer, _computeTexture->GetWidth() / 8, _computeTexture->GetHeight() / 8, 1);
        _device.EndSingleTimeCommands(cmdBuffer);
        _computeTexture->TransitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkResetCommandBuffer(_commandBuffers[imageIndex], 0);
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

            CreateOnScreenPipelines();
        }
    }

    void Tracer::RecordCommandBuffer(int index)
    {
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
        
        _graphicsPipeline->Bind(_commandBuffers[index], index);
        //_model->Bind(_commandBuffers[index]);
        vkCmdDraw(_commandBuffers[index], 6, 1, 0, 0);

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