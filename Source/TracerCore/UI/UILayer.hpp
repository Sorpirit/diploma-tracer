#pragma once

#include "../VulkanDevice.hpp"
#include "../Window.hpp"
#include "../SwapChain.hpp"

namespace TracerCore {
namespace UI
{

    class UILayer
    {
    public:
        UILayer(VulkanDevice& device)
         : _device(device), _instance(device.GetVkInstance()), _physicalDevice(device.GetPhysicalDevice()) {}
        virtual ~UILayer() = default;

        UILayer(const UILayer&) = delete;
        UILayer operator=(const UILayer&) = delete;

        virtual void Init(Window* window, SwapChain* swapchain) = 0;
        virtual void Render(VkCommandBuffer comandBuffer) = 0;

    protected:
        VkInstance _instance;
        VkPhysicalDevice _physicalDevice;
        VulkanDevice& _device;
    };

}}
