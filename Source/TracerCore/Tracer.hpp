#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Window.hpp"
#include "VulkanDevice.hpp"
#include "GraphicsPipelineObject.hpp"
#include "SwapChain.hpp"
#include "Model.hpp"
#include "Texture2D.hpp"
#include "Raytracer.hpp"
#include "ShaderReosuceManager.hpp"

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
        void CreatePipelineLayout();
        void CreatePipeline();
        void DrawFrame();
        void RecreateSwapChain();
        void CreateCommandBuffers();
        void RecordCommandBuffer(int index);
        void FreeCommandBuffers();

        Window _mainWindow{WIDTH, HEIGHT, "Sorpirit Raytracer"};
        VulkanDevice _device{_mainWindow};

        std::unique_ptr<SwapChain> _swapChain;
        std::unique_ptr<GraphicsPipelineObject> _pipline;
        std::unique_ptr<GraphicsPipelineObject> _onScreenPipline;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkCommandBuffer> _commandBuffers;
        Raytracer _raytracer;

        std::unique_ptr<Model> _model;
        std::unique_ptr<Texture2D> _texture2d;
        std::unique_ptr<Texture2D> _computeTexture;
        std::unique_ptr<ShaderReosuceManager> _shaderResourceManager;
    };
    
}