#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Window.hpp"
#include "VulkanDevice.hpp"
#include "PiplineObject.hpp"
#include "SwapChain.hpp"

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
        void CreatePipelineLayout();
        void CreatePipeline();
        void CreateCommandBuffers();
        void DrawFrame();

        Window _mainWindow{WIDTH, HEIGHT, "Hello Vulkan"};
        VulkanDevice _device{_mainWindow};
        SwapChain _swapChain{_device, _mainWindow.GetExtent()};

        std::unique_ptr<PipelineObject> _pipline;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkCommandBuffer> _commandBuffers;

        //PipelineObject _pipline{_device, PipelineObject::GetDefaultConfiguration(WIDTH, HEIGHT), "PrecompiledShaders\\Flat.vert.spv", "PrecompiledShaders\\Flat.frag.spv"};
    };
    
}