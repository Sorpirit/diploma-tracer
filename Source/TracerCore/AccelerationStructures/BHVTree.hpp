#pragma once

#include "AccelerationStructure.hpp"
#include <VulkanDevice.hpp>
#include <Models/TracerVertex.hpp>
#include <Resources/VulkanBuffer.hpp>
#include <Math/AABB.hpp>
#include "../TracerScene.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <memory>


namespace TracerCore::AccelerationStructures 
{
    struct BHVNode 
    {
        alignas(16) glm::vec3 aabbMin;
        alignas(16) glm::vec3 aabbMax;
        // if indeciesCount > 0 then this is a leaf node so nextIndex means offset in the indecies buffer. If indeciesCount == 0 then this is a branch node so nextIndex means offset in the nodes buffer
        alignas(4) uint32_t nextIndex;
        alignas(4) uint32_t indeciesCount;
    };

    struct BHVBin
    {
        AABB aabb;
        uint32_t primitiveCount = 0;
    };

    class BHVTree : public AccelerationStructure 
    {
        public:
            BHVTree(VulkanDevice& _device, AccHeruishitcType accHeruishitcType, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices);
            ~BHVTree() override;

            BHVTree(const BHVTree&) = delete;
            BHVTree operator=(const BHVTree&) = delete;

            inline const Resources::VulkanBuffer* GetNodesBuffer() const override { return _nodesBuffer.get(); }
            inline virtual const Resources::VulkanBuffer* GetIndicesBuffer() const override { return _indeciesBuffer.get(); }
        private:
            void InsertNode(BHVNode& node, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices);
            void SubdivideNode(uint32_t parentNodeIndex, std::vector<glm::vec3> allCentroids, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices, uint32_t depth);

            bool FindBestSplitPosition(const BHVNode& node, std::vector<glm::vec3> allCentroids, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices, float& bestCost, uint32_t& bestSplitAxis, float& bestSplitPos);
            float CalculateSAHNodeCost(const BHVNode& node);

            const AccHeruishitcType _accHeruishitcType;
            
            std::unique_ptr<Resources::VulkanBuffer> _nodesBuffer;
            std::unique_ptr<Resources::VulkanBuffer> _indeciesBuffer;

            std::vector<BHVNode> _nodes;
            uint32_t _nodeCount = 0;
    };

}