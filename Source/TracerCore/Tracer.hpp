#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Window.hpp"
#include "VulkanDevice.hpp"
#include "SwapChain.hpp"
#include "UI/ImguiLayer.hpp"
#include "UI/UILayer.hpp"
#include "Model.hpp"
#include "Resources/Texture2D.hpp"
#include "Resources/VulkanBuffer.hpp"
#include "Raytracer.hpp"
#include "ShaderReosuceManager.hpp"
#include "PipelineManager.hpp"
#include "PipelineObject.hpp"
#include "TracerCamera.hpp"
#include "TracerIO.hpp"
#include "UI/StatisicUILayer.hpp"
#include "UI/FrameControllsUI.hpp"
#include "TracerScene.hpp"

namespace TracerCore
{
    struct FrameData
    {
        glm::mat4x4 Projection;
        glm::mat4x4 View;
        glm::mat4x4 InvProjection;
        glm::mat4x4 InvView;  
        
        alignas(16) glm::vec3 Color;
        alignas(4) uint32_t FrameIndex;

        alignas(4) uint32_t UseAccumTexture;
        alignas(4) uint32_t AccumFrameIndex; 

        alignas(4) uint32_t BounceCount;

        alignas(4) float DefocusDiskAngle;
        alignas(4) float DefocusDiskU;
        alignas(4) float DefocusDiskV;

        //scene data
        alignas(16) glm::vec3 aabbMin;
        alignas(16) glm::vec3 aabbMax;
    };

    class Tracer
    {
    public:
        static constexpr int WIDTH = 1024;
        static constexpr int HEIGHT = 1024;

        Tracer();
        ~Tracer();

        Tracer(const Tracer&) = delete;
        Tracer &operator=(const Tracer&) = delete;

        void Run();
    private:
        void LoadModels();
        void LoadImages();
        void SwitchRaytracePipeline();
        void CreateBuffers();
        
        void CreateOnScreenPipelines();
        void CreateComputePipelines();

        void DrawFrame();
        void RecreateSwapChain();
        void CreateCommandBuffers();
        void RecordCommandBuffer(int index);
        void FreeCommandBuffers();

        Window _mainWindow{WIDTH, HEIGHT, "Sorpirit Raytracer"};
        VulkanDevice _device{_mainWindow};
        ShaderReosuceManager _shaderResourceManager{_device};
        PipelineManager _pipelineManager{_device};

        TracerScene _scene{_device};

        TracerCamera _camera;
        UI::SceneData _sceneData;
        FrameData _frameData;
        UI::FrameStatisics _frameStats;
        MaterialsSettings _materialsSettings;
        void* _frameDataPtr;

        std::unique_ptr<SwapChain> _swapChain;
        std::vector<VkCommandBuffer> _commandBuffers;

        std::unique_ptr<UI::UILayer> _uiLayer;

        std::unique_ptr<Resources::Texture2D> _texture2d;
        
        std::unique_ptr<Resources::Texture2D> _computeTexture;
        std::unique_ptr<Resources::Texture2D> _accumulationTexture;

        std::unique_ptr<Resources::VulkanBuffer> _framedataBuffer;
        std::unique_ptr<Resources::VulkanBuffer> _triangleBuffer;

        std::unique_ptr<PipelineObject> _graphicsPipeline;
        std::unique_ptr<PipelineObject> _rayTracingPipeline;
    };
    
}