
#pragma once

#include "VulkanDevice.hpp"
#include "Models/TracerVertex.hpp"
#include "Resources/VulkanBuffer.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace TracerCore
{

    struct KdTreeBounds
    {
        glm::vec3 aabbMin;
        glm::vec3 aabbMax;

        KdTreeBounds() : aabbMin(glm::vec3(FLT_MAX)), aabbMax(glm::vec3(-FLT_MAX)) {}

        void AddVertex(glm::vec3 vertex)
        {
            aabbMin = glm::min(aabbMin, vertex);
            aabbMax = glm::max(aabbMax, vertex);
        }

        void AddBounds(KdTreeBounds bounds)
        {
            aabbMin = glm::min(aabbMin, bounds.aabbMin);
            aabbMax = glm::max(aabbMax, bounds.aabbMax);
        }
    };
    

    struct KdNode {
        alignas(4) uint32_t flags;
        alignas(4) float split;
        
        alignas(4) uint32_t left;
        // uint32_t right = left + 1; as they are stored consequtevly

        alignas(4) uint32_t startIndex;
        alignas(4) uint32_t indeciesCount;
    };

    class KdTree
    {
    public:
        KdTree(VulkanDevice& _device, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices);
        ~KdTree();

        KdTree(const KdTree&) = delete;
        KdTree operator=(const KdTree&) = delete;
    private:
        void BuildTree(
            int nodeIndex, 
            const KdTreeBounds& bounds, 
            std::vector<KdTreeBounds>& boundsList,
            uint32_t* indicesList,
            int primitivesCount,
            uint32_t depth
        );

        VulkanDevice& _device;
    
        KdTreeBounds _rootBounds;
        std::vector<KdNode> _nodes;
        std::vector<uint32_t> _indices;
    };
    
} // namespace TracerCore
