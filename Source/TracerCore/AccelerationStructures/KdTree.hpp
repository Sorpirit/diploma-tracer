
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
    enum KdNodeFlags
    {
        KdNodeFlags_InnerXSplit = 0,
        KdNodeFlags_InnerYSplit = 1,
        KdNodeFlags_InnerZSplit = 2,
        KdNodeFlags_Leaf = 3,
    };

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

    class KdTree : public AccelerationStructure
    {
    public:
        KdTree(VulkanDevice& _device, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices);
        ~KdTree() override;

        KdTree(const KdTree&) = delete;
        KdTree operator=(const KdTree&) = delete;

        inline const Resources::VulkanBuffer* GetNodesBuffer() const override { return _nodesBuffer.get(); }
        inline const Resources::VulkanBuffer* GetIndicesBuffer() const override { return _indecieBuffer.get(); }

        inline const glm::vec3& GetAABBMin() const { return _rootBounds.aabbMin; }
        inline const glm::vec3& GetAABBMax() const { return _rootBounds.aabbMax; }

    private:
        void BuildTree(
            int nodeIndex, 
            const KdTreeBounds &nodeBounds, 
            const std::vector<KdTreeBounds> &allPrimitiveBounds, 
            const std::vector<uint32_t>* primitiveIndices,
            uint32_t depth
        );

        std::unique_ptr<Resources::VulkanBuffer> _nodesBuffer;
        std::unique_ptr<Resources::VulkanBuffer> _indecieBuffer;

        KdTreeBounds _rootBounds;
        std::vector<KdNode> _nodes;
        std::vector<uint32_t> _indices;
    };
    
} // namespace TracerCore
