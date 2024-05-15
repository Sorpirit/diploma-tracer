#include "Raytracer.hpp"

namespace TracerCore
{
    void Raytracer::LoadRaytracer(VulkanDevice& device)
    {
        device.initRayTracing(&_rtProperties);
    }
} // namespace TracerCore
