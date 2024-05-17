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
#include "Raytracer.hpp"
#include "ShaderReosuceManager.hpp"
#include "PipelineManager.hpp"
#include "PipelineObject.hpp"

namespace TracerCore
{
    class Tracer
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        Tracer();
        ~Tracer();

        Tracer(const Tracer&) = delete;
        Tracer &operator=(const Tracer&) = delete;

        void Run();
    private:
        void LoadModels();
        void LoadImages();
        
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
        Raytracer _raytracer;

        std::unique_ptr<SwapChain> _swapChain;
        std::vector<VkCommandBuffer> _commandBuffers;

        std::unique_ptr<UI::UILayer> _uiLayer;

        std::unique_ptr<Model> _model;

        std::unique_ptr<Resources::Texture2D> _texture2d;
        std::unique_ptr<Resources::Texture2D> _computeTexture;

        std::unique_ptr<PipelineObject> _graphicsPipeline;
        std::unique_ptr<PipelineObject> _computePipeline;
    };
    
}