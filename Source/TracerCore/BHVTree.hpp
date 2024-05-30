#pragma once

#include "VulkanDevice.hpp"
#include "Models/TracerVertex.hpp"
#include "Resources/VulkanBuffer.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <memory>


namespace TracerCore {
    struct BHVNode {
        alignas(16) glm::vec3 aabbMin;
        alignas(16) glm::vec3 aabbMax;
        alignas(4) uint32_t left;
        // uint32_t right = left + 1; as they are stored consequtevly
        alignas(4) uint32_t startIndex;
        alignas(4) uint32_t indeciesCount;
    };

    class BHVTree {
        public:
            BHVTree(VulkanDevice& _device, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices);
            ~BHVTree();

            BHVTree(const BHVTree&) = delete;
            BHVTree operator=(const BHVTree&) = delete;

            inline const Resources::VulkanBuffer* GetNodesBuffer() const { return _nodesBuffer.get(); }
        private:
            void InsertNode(BHVNode& node, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices);
            void SubdivideNode(uint32_t parentNodeIndex, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices, uint32_t depth);

            VulkanDevice& _device;
            
            std::unique_ptr<Resources::VulkanBuffer> _nodesBuffer;

            std::vector<BHVNode> _nodes;
            uint32_t _nodeCount;
    };

}