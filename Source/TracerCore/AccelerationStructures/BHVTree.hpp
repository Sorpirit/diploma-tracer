#pragma once

#include "AccelerationStructure.hpp"
#include <VulkanDevice.hpp>
#include <Models/TracerVertex.hpp>
#include <Resources/VulkanBuffer.hpp>

#include <glm/glm.hpp>
#include <vector>
#include <memory>


namespace TracerCore::AccelerationStructures 
{
    struct BHVNode 
    {
        alignas(16) glm::vec3 aabbMin;
        alignas(16) glm::vec3 aabbMax;
        alignas(4) uint32_t left;
        // uint32_t right = left + 1; as they are stored consequtevly
        alignas(4) uint32_t startIndex;
        alignas(4) uint32_t indeciesCount;
    };

    class BHVTree : public AccelerationStructure 
    {
        public:
            BHVTree(VulkanDevice& _device, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices);
            ~BHVTree() override;

            BHVTree(const BHVTree&) = delete;
            BHVTree operator=(const BHVTree&) = delete;

            inline const Resources::VulkanBuffer* GetNodesBuffer() const override { return _nodesBuffer.get(); }
            inline virtual const Resources::VulkanBuffer* GetIndicesBuffer() const override { return _indeciesBuffer.get(); }
        private:
            void InsertNode(BHVNode& node, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices);
            void SubdivideNode(uint32_t parentNodeIndex, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices, uint32_t depth);
            
            std::unique_ptr<Resources::VulkanBuffer> _nodesBuffer;
            std::unique_ptr<Resources::VulkanBuffer> _indeciesBuffer;

            std::vector<BHVNode> _nodes;
            uint32_t _nodeCount;
    };

}