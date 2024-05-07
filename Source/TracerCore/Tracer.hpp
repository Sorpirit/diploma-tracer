#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Window.hpp"
#include "VulkanDevice.hpp"
#include "PiplineObject.hpp"
#include "SwapChain.hpp"
#include "Model.hpp"
#include "Raytracer.hpp"

namespace TraceCore
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
        void CreatePipelineLayout();
        void CreatePipeline();
        void DrawFrame();
        void RecreateSwapChain();
        void CreateCommandBuffers();
        void RecordCommandBuffer(int index);
        void FreeCommandBuffers();

        Window _mainWindow{WIDTH, HEIGHT, "Hello Vulkan"};
        VulkanDevice _device{_mainWindow};

        std::unique_ptr<SwapChain> _swapChain;
        std::unique_ptr<PipelineObject> _pipline;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkCommandBuffer> _commandBuffers;
        Raytracer _raytracer;

        std::unique_ptr<Model> _model;
    };
    
}