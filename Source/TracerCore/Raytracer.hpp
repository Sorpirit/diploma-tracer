#pragma once

// vulkan headers
#include <vulkan/vulkan.h>

#include "VulkanDevice.hpp"

namespace TracerCore
{
    class Raytracer
    {
    public:
        void LoadRaytracer(VulkanDevice& device);

    private:
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR _rtProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
    };
} // namespace TracerCore
