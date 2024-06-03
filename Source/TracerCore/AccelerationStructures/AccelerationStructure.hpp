#pragma once

#include <VulkanDevice.hpp>
#include <Models/TracerVertex.hpp>
#include <Resources/VulkanBuffer.hpp>

namespace TracerCore::AccelerationStructures {

    class AccelerationStructure
    {
    public:
        AccelerationStructure(VulkanDevice& _device, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices) : _device(_device) { }
        virtual ~AccelerationStructure() { }

        inline virtual const Resources::VulkanBuffer* GetNodesBuffer() const = 0;
        inline virtual const Resources::VulkanBuffer* GetIndicesBuffer() const = 0;

    protected:
        VulkanDevice& _device;
    };
    

} // namespace TracerCore::AccelerationStructures