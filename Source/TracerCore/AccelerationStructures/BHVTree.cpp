#include "BHVTree.hpp"

namespace TracerCore::AccelerationStructures
{
    BHVTree::BHVTree(VulkanDevice &_device, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices) : 
        AccelerationStructure(_device, vertices, indices)
    {
        std::vector<uint32_t> indicesCopy;
        indicesCopy.insert(indicesCopy.begin(), indices.begin(), indices.end()); 

        std::vector<glm::vec3> centroids;
        for (size_t i = 0; i < indicesCopy.size(); i+=3)
        {
            glm::vec3 centroid = (vertices[indicesCopy[i]].Position + vertices[indicesCopy[i + 1]].Position + vertices[indicesCopy[i + 2]].Position) * 0.333333f;
            centroids.push_back(centroid);
        }

        BHVNode rootNode;
        rootNode.aabbMin = glm::vec3(0.0f);
        rootNode.aabbMax = glm::vec3(0.0f);
        rootNode.nextIndex = 0;
        rootNode.indeciesCount = indicesCopy.size();
        InsertNode(rootNode, vertices, indicesCopy);
        SubdivideNode(0, vertices, indicesCopy, 1);

        _nodesBuffer = Resources::VulkanBuffer::CreateBuffer(_device, sizeof(BHVNode) * _nodes.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        _indeciesBuffer = Resources::VulkanBuffer::CreateBuffer(_device, sizeof(uint32_t) * indicesCopy.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        _nodesBuffer->MapMemory(sizeof(BHVNode) * _nodes.size(), 0, &data);
        memcpy(data, _nodes.data(), sizeof(BHVNode) * _nodes.size());
        _nodesBuffer->UnmapMemory();

        _indeciesBuffer->MapMemory(sizeof(uint32_t) * indicesCopy.size(), 0, &data);
        memcpy(data, indicesCopy.data(), sizeof(uint32_t) * indicesCopy.size());
        _indeciesBuffer->UnmapMemory();
    }

    BHVTree::~BHVTree()
    {
    }

    void BHVTree::InsertNode(BHVNode &node, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices)
    {
        node.aabbMax = glm::vec3(-FLT_MAX);
        node.aabbMin = glm::vec3(FLT_MAX);

        for (size_t i = 0; i < node.indeciesCount; i++)
        {
            uint32_t index = indices[node.nextIndex + i];
            const auto vertex = vertices[index].Position;
            node.aabbMin = glm::min(node.aabbMin, vertex);
            node.aabbMax = glm::max(node.aabbMax, vertex);
        }

        _nodeCount++;
        _nodes.push_back(node);
    }

    void BHVTree::SubdivideNode(uint32_t parentNodeIndex, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices, uint32_t depth)
    {
        if(depth > 64)
            return;

        BHVNode& parentNode = _nodes[parentNodeIndex];
        glm::vec3 aabbSize = parentNode.aabbMax - parentNode.aabbMin;
        float parentSurfaceArea = 2.0f * (aabbSize.x * aabbSize.y + aabbSize.x * aabbSize.z + aabbSize.y * aabbSize.z);
        float parentCost = (parentNode.indeciesCount / 3) * parentSurfaceArea;

        int bestAxis = -1;
        float bestCost = FLT_MAX;
        float bestSplitPos = 0.0f;

        //SAH
        // {
        //     for (size_t axis = 0; axis < 3; axis++)
        //     {
        //         for (size_t j = 0; j < parentNode.indeciesCount; j+=3)
        //         {
        //             uint32_t currentIndecie = parentNode.nextIndex + j;
        //             glm::vec3 triCenter = 0.333333f * (
        //                 vertices[indices[currentIndecie]].Position + 
        //                 vertices[indices[currentIndecie + 1]].Position + 
        //                 vertices[indices[currentIndecie + 2]].Position);

        //             float splitPos = triCenter[axis];
        //             float cost = CalculateSAH(parentNodeIndex, axis, splitPos, vertices, indices);
        //             if(cost < bestCost)
        //             {
        //                 bestCost = cost;
        //                 bestAxis = axis;
        //                 bestSplitPos = splitPos;
        //             }
        //         }
        //     }

        //     if(bestCost >= parentCost)
        //     {
        //         return;
        //     }
        // }

        //Prmitive
        {
            bestAxis = 0;
            if(aabbSize.y > aabbSize.x && aabbSize.y > aabbSize.z)
            {
                bestAxis = 1;
            }
            else if(aabbSize.z > aabbSize.x && aabbSize.z > aabbSize.y)
            {
                bestAxis = 2;
            }

            bestSplitPos = parentNode.aabbMin[bestAxis] + aabbSize[bestAxis] * 0.5f;

            if(parentNode.indeciesCount <= 10)
            {
                return;
            }
        }

        int currentIndecie = parentNode.nextIndex;
        int lastIndecie = parentNode.nextIndex + parentNode.indeciesCount - 1;
        while (currentIndecie <= lastIndecie)
        {
            glm::vec3 triCenter = 0.333333f * (
                vertices[indices[currentIndecie]].Position + 
                vertices[indices[currentIndecie + 1]].Position + 
                vertices[indices[currentIndecie + 2]].Position);
            if(triCenter[bestAxis] < bestSplitPos)
            {
                currentIndecie+=3;
            }
            else
            {
                std::swap(indices[currentIndecie], indices[lastIndecie - 2]);
                std::swap(indices[currentIndecie + 1], indices[lastIndecie - 1]);
                std::swap(indices[currentIndecie + 2], indices[lastIndecie]);
                lastIndecie-=3;
            }
        }

        int leftCount = currentIndecie - parentNode.nextIndex;
        if(leftCount == 0 || leftCount == parentNode.indeciesCount)
        {
            return;
        }

        BHVNode leftNode;
        uint32_t leftNodeId = _nodeCount;
        leftNode.nextIndex = parentNode.nextIndex;
        leftNode.indeciesCount = leftCount;

        BHVNode rightNode;
        uint32_t rightIndexId = _nodeCount + 1;
        rightNode.nextIndex = currentIndecie;
        rightNode.indeciesCount = parentNode.indeciesCount - leftCount;

        parentNode.indeciesCount = 0;
        parentNode.nextIndex = _nodeCount;

        InsertNode(leftNode, vertices, indices);
        InsertNode(rightNode, vertices, indices);

        SubdivideNode(leftNodeId, vertices, indices, depth + 1);
        SubdivideNode(rightIndexId, vertices, indices, depth + 1);
    }

    float BHVTree::CalculateSAH(uint32_t nodeIndex, int splitAxis, float splitPos, std::vector<TracerUtils::Models::TracerVertex> &vertices, std::vector<uint32_t> &indices)
    {
        BHVNode& node = _nodes[nodeIndex];

        AABB leftAABB;
        uint32_t leftCount = 0;

        AABB rightAABB;
        uint32_t rightCount = 0;

        for (size_t i = 0; i < node.indeciesCount; i+=3)
        {
            uint32_t currentIndecie = node.nextIndex + i;
            glm::vec3 v1 = vertices[indices[currentIndecie]].Position;
            glm::vec3 v2 = vertices[indices[currentIndecie + 1]].Position;
            glm::vec3 v3 = vertices[indices[currentIndecie + 2]].Position;

            glm::vec3 triCenter = 0.333333f * (v1 + v2 + v3);

            if(triCenter[splitAxis] < splitPos)
            {
                leftAABB.Expand(v1);
                leftAABB.Expand(v2);
                leftAABB.Expand(v3);
                leftCount++;
            }
            else
            {
                rightAABB.Expand(v1);
                rightAABB.Expand(v2);
                rightAABB.Expand(v3);
                rightCount++;
            }
        }
        
        float cost = leftCount * leftAABB.SurfaceArea() + rightCount * rightAABB.SurfaceArea();
        return cost;
    }
}