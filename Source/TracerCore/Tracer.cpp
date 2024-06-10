#include "Tracer.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

#include <tracy/Tracy.hpp>

#include "UI/CamerUIControl.hpp"

#include "Models/TracerVertex.hpp"



namespace TracerCore
{
    //test animation
    static const bool AnimatedCamera = false;
    static const glm::vec3 cameraPositions[] = {
        glm::vec3(3, .4, .4),
        glm::vec3(0, .7, -4.0),
        glm::vec3(0, 3, 0),
        glm::vec3(-2.4f, 0.7f, 3.5f),
        glm::vec3(0, 0.5f, 4.0f),
    };
    static const float timePreTransition = 8.0f;
    static int currentCameraPosition = 0;
    static float timeTransition = 0.0f;
    

    static void AnimateCamera(TracerCamera& camera, float deltaTime, uint32_t frameCount)
    {
        glm::vec3 position = glm::lerp(cameraPositions[currentCameraPosition], cameraPositions[(currentCameraPosition + 1) % 5], timeTransition / timePreTransition);
        if(frameCount > 10)
        {
            if(timeTransition < timePreTransition)
            {
                timeTransition += deltaTime;
            }
            else
            {
                timeTransition = 0.0f;
                currentCameraPosition = (currentCameraPosition + 1) % 5;
            }

            camera.SetParameters(position, glm::normalize(glm::vec3(0, .5, 0) - position));
            camera.SetStatic(false);
        }
    }

    Tracer::Tracer()
    {
        ZoneScoped;

        auto extent = _mainWindow.GetExtent();
        glm::vec3 position = cameraPositions[0];
        glm::vec3 target = glm::vec3(0, .5, 0);
        _camera.SetParameters(position, glm::normalize(target - position));
        _camera.SetProjection(glm::radians(95.0f), extent.width / (float) extent.height, 0.1f, 150.0f);

        _sceneData.ModelPath = "Models\\suzanne.fbx";
        _sceneData.AccStructureType = AccStructureType::AccStructure_BVH;
        _sceneData.AccHeruishitcType = AccHeruishitcType::AccHeruishitc_SAH;

        _frameStats.color = glm::vec3(0.5, 0.7, 1.0);
        _frameStats.bounceCount = 16;

        _frameData.Color = _frameStats.color;
        _frameData.FrameIndex = 1;
        _frameData.Projection = _camera.GetProjection();
        _frameData.InvProjection = _camera.GetInvProjection();
        _frameData.View = _camera.GetView();
        _frameData.InvView = _camera.GetInvView();
        _frameData.BounceCount = _frameStats.bounceCount;

        _materialsSettings.groundAlbedo = glm::vec3(0.8f, 0.8f, 0.0f);
        _materialsSettings.UseRandomMaterials = true;
        _materialsSettings.meshAlbedo = glm::vec3(0.83f, 0.65f, 0.92f);
        _materialsSettings.meshFuzz = 0.4f;


        LoadImages();
        RecreateSwapChain();

        CreateBuffers();
        CreateOnScreenPipelines();
        CreateComputePipelines();
        SwitchRaytracePipeline();
        LoadModels();
        
        CreateCommandBuffers();
    }

    Tracer::~Tracer()
    {
        _framedataBuffer->UnmapMemory();
        _uiLayer = nullptr;
    }

    void Tracer::Run()
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::cout << extensionCount << " extensions supported\n";
        uint32_t frameCount = 1;

        uint32_t lastFPSReadFrameId = 0;
        uint32_t lastFPSRead = 0;
        uint32_t frameTimeSum = 0;

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        float currentFrame = 0.0f;

        uint32_t accumStartFrameIndex = 0;

        while(_mainWindow.ShouldClose()) {
            
            currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            frameTimeSum += (uint32_t) (deltaTime * 1000.0f);
            if(frameTimeSum >= 3000.0)
            {
                frameTimeSum = 0.0f;
                lastFPSRead = (frameCount - lastFPSReadFrameId) / 3;
                lastFPSReadFrameId = frameCount;
                _frameStats.FPS = lastFPSRead;
            }

            _frameStats.FrameTime = deltaTime;

            if(AnimatedCamera)
            {
                AnimateCamera(_camera, deltaTime, frameCount);
            }

            //update frame params
            _frameData.Color = _frameStats.color;
            _frameData.Projection = _camera.GetProjection();
            _frameData.InvProjection = _camera.GetInvProjection();
            _frameData.View = _camera.GetView();
            _frameData.InvView = _camera.GetInvView();
            _frameData.FrameIndex = frameCount;
            
            if(_frameData.UseAccumTexture != _camera.IsStatic())
            {
                accumStartFrameIndex = frameCount;
            }

            _frameData.UseAccumTexture = _camera.IsStatic();
            _frameData.AccumFrameIndex = frameCount - accumStartFrameIndex;
            _frameData.BounceCount = _frameStats.bounceCount;

            VkDeviceSize size = sizeof(FrameData);
            memcpy(_frameDataPtr, &_frameData, size);
            
            glfwPollEvents();
            DrawFrame();

            if(!_sceneData.IsSceneLoaded)
            {
                SwitchRaytracePipeline();
                LoadModels();
                _camera.SetStatic(false);
            }

            if(_sceneData.ResetCamera)
            {
                glm::vec3 position = cameraPositions[0];
                glm::vec3 target = glm::vec3(0, .5, 0);
                _camera.SetParameters(position, glm::normalize(target - position));
                _sceneData.ResetCamera = false;
                _camera.SetStatic(false);
            }

            frameCount++;
            FrameMark;
        }

        vkDeviceWaitIdle(_device.GetVkDevice());
    }

    void Tracer::LoadModels()
    {
        auto model = TracerUtils::IOHelpers::LoadModel(_sceneData.ModelPath);
        _scene.AddModel(model);
        _scene.BuildMaterials(_materialsSettings);
        _scene.BuildScene(_sceneData.AccStructureType, _sceneData.AccHeruishitcType);
        std::cout << "Scene builded. " << _sceneData.ModelPath << " loaded. Acc structure:" << (int) _sceneData.AccStructureType << " Acc heuristic:" << (int) _sceneData.AccHeruishitcType << "\n";

        _frameStats.TriCount = _scene.GetIndeciesCount() / 3;
        _frameData.aabbMin = _scene.GetAABBMin();
        _frameData.aabbMax = _scene.GetAABBMax();
        _sceneData.IsSceneLoaded = true;

        _scene.AttachSceneGeometry(_shaderResourceManager, _rayTracingPipeline->GetDescriptorSets());
    }

    void Tracer::LoadImages()
    {
        // _texture2d = Resources::Texture2D::LoadFileTexture("Textures\\cutecat.jpg", _device);

        auto extent = _mainWindow.GetExtent();
        _computeTexture = Resources::Texture2D::CreateTexture2D(extent.width, extent.height, 
            VK_FORMAT_R8G8B8A8_UNORM, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT  | VK_IMAGE_USAGE_SAMPLED_BIT,
            false,
            _device
        );

        _accumulationTexture = Resources::Texture2D::CreateTexture2D(extent.width, extent.height, 
            VK_FORMAT_R32G32B32A32_SFLOAT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT  | VK_IMAGE_USAGE_SAMPLED_BIT,
            false,
            _device
        );
        _accumulationTexture->TransitionImageLayout(VK_IMAGE_LAYOUT_GENERAL);
    }

    void Tracer::SwitchRaytracePipeline()
    {
        switch (_sceneData.AccStructureType)
        {
        case AccStructureType::AccStructure_BVH:   
            _rayTracingPipeline->SetPipelineVariantIndex(0);
            break;
        case AccStructureType::AccStructure_KdTree:
            _rayTracingPipeline->SetPipelineVariantIndex(1);
            break;
        case AccStructureType::AccStructure_None:
            _rayTracingPipeline->SetPipelineVariantIndex(2);
            break;
        
        default:
            break;
        }
    }

    void Tracer::CreateBuffers()
    {
        VkDeviceSize size = sizeof(FrameData);
        _framedataBuffer = Resources::VulkanBuffer::CreateBuffer(_device, size, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        _framedataBuffer->MapMemory(size, 0, &_frameDataPtr);
        memcpy(_frameDataPtr, &_frameData, size);
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

        std::vector<VkPipeline> variants = std::vector<VkPipeline>{onScreenPipeline};

        _graphicsPipeline = std::make_unique<PipelineObject>(
            _device,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            setLayout,
            descriptorPool, 
            std::move(descriptorSets),
            pipelineLayout,
            std::move(variants)
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

        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, imageCount * 2},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, imageCount},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, imageCount * 4},
        };

        const int bindingCount = 7;
        VkDescriptorSetLayoutBinding layoutBindings[bindingCount];
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[0].pImmutableSamplers = nullptr;
                
        layoutBindings[1].binding = 1;
        layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        layoutBindings[1].descriptorCount = 1;
        layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[1].pImmutableSamplers = nullptr;

        layoutBindings[2].binding = 2;
        layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[2].descriptorCount = 1;
        layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[2].pImmutableSamplers = nullptr;
        
        layoutBindings[3].binding = 3;
        layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[3].descriptorCount = 1;
        layoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[3].pImmutableSamplers = nullptr;
        
        layoutBindings[4].binding = 4;
        layoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[4].descriptorCount = 1;
        layoutBindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[4].pImmutableSamplers = nullptr;
        
        layoutBindings[5].binding = 5;
        layoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[5].descriptorCount = 1;
        layoutBindings[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[5].pImmutableSamplers = nullptr;

        layoutBindings[6].binding = 6;
        layoutBindings[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[6].descriptorCount = 1;
        layoutBindings[6].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[6].pImmutableSamplers = nullptr;

        _shaderResourceManager.CreateDescriptorPool(poolSizes, 1, imageCount, descriptorPool);
        _shaderResourceManager.CreateDescriptorSetLayout(layoutBindings, bindingCount, setLayout);
        _shaderResourceManager.CreateDescriptorSets(descriptorPool, setLayout, imageCount, descriptorSets);

        _pipelineManager.CreatePipelineLayout(setLayout, &pipelineLayout);

        VkPipeline bhvPipeline;
        VkPipeline kdPipeline;
        VkPipeline simpleLoopPipeline;
    
        _pipelineManager.CreateComputePipeline(pipelineLayout, "PrecompiledShaders\\RaytraceKdTree.comp.spv", &kdPipeline);
        _pipelineManager.CreateComputePipeline(pipelineLayout, "PrecompiledShaders\\RaytraceBHVTree.comp.spv", &bhvPipeline);
        _pipelineManager.CreateComputePipeline(pipelineLayout, "PrecompiledShaders\\RaytraceSimpleLoop.comp.spv", &simpleLoopPipeline);

        _shaderResourceManager.UploadTexture(descriptorSets, 0, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, _computeTexture.get());
        _shaderResourceManager.UploadTexture(descriptorSets, 1, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, _accumulationTexture.get());
        _shaderResourceManager.UploadBuffer(descriptorSets, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _framedataBuffer.get());

        std::vector<VkPipeline> variants = std::vector<VkPipeline>{bhvPipeline, kdPipeline, simpleLoopPipeline};
        
        _rayTracingPipeline = std::make_unique<PipelineObject>(
            _device,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            setLayout,
            descriptorPool, 
            descriptorSets,
            pipelineLayout,
            variants
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
        {
            ZoneScopedN("ComputeRays");
            _computeTexture->TransitionImageLayout(VK_IMAGE_LAYOUT_GENERAL);
            auto cmdBuffer = _device.BeginSingleTimeCommands();
            _rayTracingPipeline->Bind(cmdBuffer, imageIndex);
            vkCmdDispatch(cmdBuffer, glm::ceil( _computeTexture->GetWidth() / 32.0f), glm::ceil(_computeTexture->GetHeight() / 32.0f), 1);
            _device.EndSingleTimeCommands(cmdBuffer);
            _computeTexture->TransitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

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
            CreateComputePipelines();
        }

        _uiLayer = nullptr;
        _uiLayer = std::make_unique<UI::ImguiLayer>(_device);
        _uiLayer->Init(&_mainWindow, _swapChain.get());
        _uiLayer->AddLayer(std::make_unique<UI::CamerUIControl>(_camera, _mainWindow));
        _uiLayer->AddLayer(std::make_unique<UI::StatisticsWindow>(_frameStats));
        _uiLayer->AddLayer(std::make_unique<UI::FrameControllsUI>(_device, _mainWindow, _sceneData, _computeTexture.get()));
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

        _uiLayer->Render(_commandBuffers[index]);

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