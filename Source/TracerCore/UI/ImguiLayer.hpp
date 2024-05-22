#pragma once

#include "UILayer.hpp"
#include "../VulkanDevice.hpp"


namespace TracerCore {
namespace UI
{

    class ImguiLayer : public UILayer
    {
    public:
        ImguiLayer(VulkanDevice& device);
        ~ImguiLayer();

        ImguiLayer(const ImguiLayer&) = delete;
        ImguiLayer operator=(const ImguiLayer&) = delete;

        void Init(Window* window, SwapChain* swapchain) override;
        void Render(VkCommandBuffer comandBuffer) override;

    private:
        bool _showDemoWindow = true;
        bool _isInitilized = false;

        VkDescriptorPool _descriptorPool;
    };

}}