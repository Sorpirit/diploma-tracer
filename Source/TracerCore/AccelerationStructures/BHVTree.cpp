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
        SubdivideNode(0, centroids, vertices, indicesCopy, 1);

        _nodesBuffer = Resources::VulkanBuffer::CreateBuffer(_device, sizeof(BHVNode) * _nodes.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        _indeciesBuffer = Resources::VulkanBuffer::CreateBuffer(_device, sizeof(uint32_t) * indicesCopy.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        _nodesBuffer->MapMemory(sizeof(BHVNode) * _nodes.size(), 0, &data);
        memcpy(data, _nodes.data(), sizeof(BHVNode) * _nodes.size());
        _nodesBuffer->UnmapMemory();

        _indeciesBuffer->MapMemory(sizeof(uint32_t) * indicesCopy.size(), 0, &data);
        memcpy(data, indicesCopy.data(), sizeof(uint32_t) * indicesCopy.size());
        _indeciesBuffer->UnmapMemory();

        _indeciesCount = indicesCopy.size();
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

    void BHVTree::SubdivideNode(uint32_t parentNodeIndex, std::vector<glm::vec3> allCentroids, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices, uint32_t depth)
    {
        if(depth > 32)
            return;

        BHVNode& parentNode = _nodes[parentNodeIndex];
        uint32_t bestAxis = -1;
        float bestCost = FLT_MAX;
        float bestSplitPos = 0.0f;

        if(!FindBestSplitPosition(parentNode, allCentroids, vertices, indices, bestCost, bestAxis, bestSplitPos))
        {
            return;
        }

        int currentIndecie = parentNode.nextIndex;
        int lastIndecie = parentNode.nextIndex + parentNode.indeciesCount - 1;
        while (currentIndecie <= lastIndecie)
        {
            glm::vec3 triCenter = allCentroids[currentIndecie / 3];
            if(triCenter[bestAxis] < bestSplitPos)
            {
                currentIndecie+=3;
            }
            else
            {
                std::swap(indices[currentIndecie], indices[lastIndecie - 2]);
                std::swap(indices[currentIndecie + 1], indices[lastIndecie - 1]);
                std::swap(indices[currentIndecie + 2], indices[lastIndecie]);
                std::swap(allCentroids[currentIndecie / 3], allCentroids[lastIndecie / 3]);
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

        SubdivideNode(leftNodeId, allCentroids, vertices, indices, depth + 1);
        SubdivideNode(rightIndexId, allCentroids, vertices, indices, depth + 1);
    }

    bool BHVTree::FindBestSplitPosition(const BHVNode& node, std::vector<glm::vec3> allCentroids, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices, float& bestCost, uint32_t& bestSplitAxis, float& bestSplitPos)
    {
        // SAH Full
        // {
        //     float parentCost = CalculateSAHNodeCost(node);

        //     for (uint32_t axis = 0; axis < 3; axis++)
        //     {
        //         for (uint32_t j = 0; j < node.indeciesCount; j+=3)
        //         {
        //             uint32_t currentIndecie = node.nextIndex + j;
        //             glm::vec3 triCenter = allCentroids[currentIndecie / 3];

        //             float splitPos = triCenter[axis];
        //             float cost = CalculateSAH(node, axis, splitPos, bestCost, allCentroids, vertices, indices);
        //             if(cost < bestCost)
        //             {
        //                 bestCost = cost;
        //                 bestSplitAxis = axis;
        //                 bestSplitPos = splitPos;
        //             }
        //         }
        //     }

        //     if(bestCost >= parentCost)
        //     {
        //         return false;
        //     }
        // }

        // SAH Scaled aproximation
        {
            if(node.indeciesCount <= 32)
            {
                return false;
            }

            const uint32_t BINS = 8;
            float parentCost = CalculateSAHNodeCost(node);

            for (uint32_t axis = 0; axis < 3; axis++)
            {
                float boundsMin = node.aabbMin[axis];
                float boundsMax = node.aabbMax[axis];
                if((boundsMax - boundsMin) < 0.0001f)
                    continue;

                for (size_t i = 0; i < node.indeciesCount; i+=3)
                {
                    uint32_t currentIndecie = node.nextIndex + i;
                    glm::vec3 triCenter = allCentroids[currentIndecie / 3];
                    boundsMin = glm::min(boundsMin, triCenter[axis]);
                    boundsMax = glm::max(boundsMax, triCenter[axis]);
                }

                BHVBin bins[BINS];
                float scale = BINS / (boundsMax - boundsMin);
                for (size_t i = 0; i < node.indeciesCount; i+=3)
                {
                    uint32_t currentIndecie = node.nextIndex + i;
                    glm::vec3 v1 = vertices[indices[currentIndecie]].Position;
                    glm::vec3 v2 = vertices[indices[currentIndecie + 1]].Position;
                    glm::vec3 v3 = vertices[indices[currentIndecie + 2]].Position;
                    glm::vec3 triCenter = allCentroids[currentIndecie / 3];

                    int binIndex = __min(BINS - 1, (int) ((triCenter[axis] - boundsMin) * scale));
                    bins[binIndex].aabb.Expand(v1);
                    bins[binIndex].aabb.Expand(v2);
                    bins[binIndex].aabb.Expand(v3);
                    bins[binIndex].primitiveCount++;
                }

                float leftAreas[BINS - 1];
                float rightAreas[BINS - 1];

                int leftCount[BINS - 1];
                int rightCount[BINS - 1];

                AABB leftAABB;
                uint32_t leftTriCount = 0;
                AABB rightAABB;
                uint32_t rightTriCount = 0;
                for (size_t i = 0; i < (BINS - 1); i++)
                {
                    leftTriCount += bins[i].primitiveCount;
                    leftCount[i] = leftTriCount;
                    leftAABB.Expand(bins[i].aabb);
                    leftAreas[i] = leftAABB.SurfaceArea();
                    
                    rightTriCount += bins[BINS - 1 - i].primitiveCount;
                    rightCount[BINS - 2 - i] = rightTriCount;
                    rightAABB.Expand(bins[BINS - 1 - i].aabb);
                    rightAreas[BINS - 2 - i] = rightAABB.SurfaceArea();
                }
                
                scale = (boundsMax - boundsMin) / BINS;
                for (size_t i = 0; i < (BINS - 1); i++)
                {
                    float cost = leftAreas[i] * leftCount[i] + rightAreas[i] * rightCount[i];
                    if(cost < bestCost)
                    {
                        bestCost = cost;
                        bestSplitAxis = axis;
                        bestSplitPos = boundsMin + scale * (i + 1);
                    }
                }
            }

            if(bestCost < 1.0f || bestCost >= parentCost)
            {
                return false;
            }
        }
        
        //Prmitive
        // {
        //     if(node.indeciesCount <= 32)
        //     {
        //         return false;
        //     }

        //     glm::vec3 aabbSize = node.aabbMax - node.aabbMin;
        //     bestSplitAxis = 0;
        //     if(aabbSize.y > aabbSize.x && aabbSize.y > aabbSize.z)
        //     {
        //         bestSplitAxis = 1;
        //     }
        //     else if(aabbSize.z > aabbSize.x && aabbSize.z > aabbSize.y)
        //     {
        //         bestSplitAxis = 2;
        //     }

        //     bestSplitPos = node.aabbMin[bestSplitAxis] + aabbSize[bestSplitAxis] * 0.5f;
        // }

        return true;
    }

    float BHVTree::CalculateSAHNodeCost(const BHVNode &node)
    {
        glm::vec3 aabbSize = node.aabbMax - node.aabbMin;
        float parentSurfaceArea = 2.0f * (aabbSize.x * aabbSize.y + aabbSize.x * aabbSize.z + aabbSize.y * aabbSize.z);
        return (node.indeciesCount / 3) * parentSurfaceArea;
    }
}