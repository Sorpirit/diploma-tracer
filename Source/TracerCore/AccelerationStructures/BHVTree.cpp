#include "BHVTree.hpp"

namespace TracerCore::AccelerationStructures
{
    BHVTree::BHVTree(VulkanDevice &_device, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices) : 
        AccelerationStructure(_device, vertices, indices)
    {
        std::vector<uint32_t> indicesCopy;
        indicesCopy.insert(indicesCopy.begin(), indices.begin(), indices.end()); 

        BHVNode rootNode;
        rootNode.aabbMin = glm::vec3(0.0f);
        rootNode.aabbMax = glm::vec3(0.0f);
        rootNode.left = 0;
        rootNode.startIndex = 0;
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
            uint32_t index = indices[node.startIndex + i];
            const auto vertex = vertices[index].Position;
            node.aabbMin = glm::min(node.aabbMin, vertex);
            node.aabbMax = glm::max(node.aabbMax, vertex);
        }

        _nodeCount++;
        _nodes.push_back(node);
    }

    void BHVTree::SubdivideNode(uint32_t parentNodeIndex, std::vector<TracerUtils::Models::TracerVertex>& vertices, std::vector<uint32_t>& indices, uint32_t depth)
    {
        BHVNode& parentNode = _nodes[parentNodeIndex];
        if(parentNode.indeciesCount <= 2 || depth > 64)
        {
            return;
        }

        glm::vec3 aabbSize = parentNode.aabbMax - parentNode.aabbMin;
        uint32_t splitAxis = 0;
        if(aabbSize.y > aabbSize.x && aabbSize.y > aabbSize.z)
        {
            splitAxis = 1;
        }
        else if(aabbSize.z > aabbSize.x && aabbSize.z > aabbSize.y)
        {
            splitAxis = 2;
        }

        float splitPos = parentNode.aabbMin[splitAxis] + aabbSize[splitAxis] * 0.5f;

        int currentIndecie = parentNode.startIndex;
        int lastIndecie = parentNode.startIndex + parentNode.indeciesCount - 1;
        while (currentIndecie <= lastIndecie)
        {
            glm::vec3 triCenter = 0.33f * (
                vertices[indices[currentIndecie]].Position + 
                vertices[indices[currentIndecie + 1]].Position + 
                vertices[indices[currentIndecie + 2]].Position);
            if(triCenter[splitAxis] < splitPos)
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

        int leftCount = currentIndecie - parentNode.startIndex;
        if(leftCount == 0 || leftCount == parentNode.indeciesCount)
        {
            return;
        }

        BHVNode leftNode;
        uint32_t leftNodeId = _nodeCount;
        leftNode.startIndex = parentNode.startIndex;
        leftNode.indeciesCount = leftCount;
        leftNode.left = 0;

        BHVNode rightNode;
        uint32_t rightIndexId = _nodeCount + 1;
        rightNode.startIndex = currentIndecie;
        rightNode.indeciesCount = parentNode.indeciesCount - leftCount;
        rightNode.left = 0;

        parentNode.indeciesCount = 0;
        parentNode.left = _nodeCount;

        InsertNode(leftNode, vertices, indices);
        InsertNode(rightNode, vertices, indices);

        SubdivideNode(leftNodeId, vertices, indices, depth + 1);
        SubdivideNode(rightIndexId, vertices, indices, depth + 1);
    }
}