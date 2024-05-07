#include "Raytracer.hpp"

namespace TraceCore
{
    void Raytracer::LoadRaytracer(VulkanDevice& device)
    {
        device.initRayTracing(&_rtProperties);
    }
} // namespace TraceCore
