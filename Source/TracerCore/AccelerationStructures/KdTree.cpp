#include "KdTree.hpp"

namespace TracerCore::AccelerationStructures
{
    KdTree::KdTree(VulkanDevice &_device, std::vector<TracerUtils::Models::TracerVertex> &vertices, std::vector<uint32_t> &indices) 
        : AccelerationStructure(_device, vertices, indices)
    {
        //Compute bounds
        std::vector<KdTreeBounds> bounds;
        std::vector<uint32_t> primitiveIndices;
        for (size_t i = 0; i < indices.size(); i+=3)
        {
            KdTreeBounds triangleBounds;
            triangleBounds.AddVertex(vertices[indices[i]].Position);
            triangleBounds.AddVertex(vertices[indices[i + 1]].Position);
            triangleBounds.AddVertex(vertices[indices[i + 2]].Position);
            _rootBounds.AddBounds(triangleBounds);

            bounds.push_back(triangleBounds);
            primitiveIndices.push_back(i / 3);
        }

        _nodes.push_back({3, 0.5f, 0, 0, 0});
        BuildTree(0, _rootBounds, bounds, &primitiveIndices, 1);

        for (size_t i = 0; i < _indices.size(); i++)
        {
            _indices[i] = indices[_indices[i]];
        }

        _nodesBuffer = Resources::VulkanBuffer::CreateBuffer(_device, sizeof(KdNode) * _nodes.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        _indecieBuffer = Resources::VulkanBuffer::CreateBuffer(_device, sizeof(uint32_t) * _indices.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        _nodesBuffer->MapMemory(sizeof(KdNode) * _nodes.size(), 0, &data);
        memcpy(data, _nodes.data(), sizeof(KdNode) * _nodes.size());
        _nodesBuffer->UnmapMemory();

        _indecieBuffer->MapMemory(sizeof(uint32_t) * _indices.size(), 0, &data);
        memcpy(data, _indices.data(), sizeof(uint32_t) * _indices.size());
        _indecieBuffer->UnmapMemory();
    }

    KdTree::~KdTree()
    {
    }

    void KdTree::BuildTree(
        int nodeIndex, 
        const KdTreeBounds &nodeBounds, 
        const std::vector<KdTreeBounds> &allPrimitiveBounds, 
        const std::vector<uint32_t>* primitiveIndices,
        uint32_t depth)
    {
        if(primitiveIndices->size() <= 10 || depth > 32)
        {
            KdNode& leafNode = _nodes[nodeIndex];
            leafNode.flags = KdNodeFlags_Leaf;
            leafNode.startIndex = _indices.size();
            leafNode.indeciesCount = primitiveIndices->size() * 3;

            for (size_t i = 0; i < primitiveIndices->size(); i++)
            {
                uint32_t indecieStatIndex = 3 * ((*primitiveIndices)[i]);
                _indices.push_back(indecieStatIndex);
                _indices.push_back(indecieStatIndex + 1);
                _indices.push_back(indecieStatIndex + 2);
            }

            return;
        }

        glm::vec3 aabbSize = nodeBounds.aabbMax - nodeBounds.aabbMin;
        uint32_t splitAxis = 0;
        if(aabbSize.y > aabbSize.x && aabbSize.y > aabbSize.z)
        {
            splitAxis = 1;
        }
        else if(aabbSize.z > aabbSize.x && aabbSize.z > aabbSize.y)
        {
            splitAxis = 2;
        }

        float splitPos = nodeBounds.aabbMin[splitAxis] + aabbSize[splitAxis] * 0.5f;

        KdTreeBounds leftChildBounds = nodeBounds;
        KdTreeBounds rightChildBounds = nodeBounds;
        leftChildBounds.aabbMax[splitAxis] = splitPos;
        rightChildBounds.aabbMin[splitAxis] = splitPos;

        auto leftIndecies = std::make_unique<std::vector<uint32_t>>();
        auto rightIndecies = std::make_unique<std::vector<uint32_t>>();

        for (size_t i = 0; i < primitiveIndices->size(); i++)
        {
            uint32_t primitiveIndex = (*primitiveIndices)[i];
            KdTreeBounds primitiveBounds = allPrimitiveBounds[primitiveIndex];
            if(primitiveBounds.aabbMax[splitAxis] < splitPos)
            {
                leftIndecies->push_back(primitiveIndex);
            }
            else if(primitiveBounds.aabbMin[splitAxis] > splitPos)
            {
                rightIndecies->push_back(primitiveIndex);
            }
            else
            {
                leftIndecies->push_back(primitiveIndex);
                rightIndecies->push_back(primitiveIndex);
            }
        }

        if (leftIndecies->size() == 0 || rightIndecies->size() == 0)
        {
            KdNode& leafNode = _nodes[nodeIndex];
            leafNode.flags = KdNodeFlags_Leaf;
            leafNode.startIndex = _indices.size();
            leafNode.indeciesCount = primitiveIndices->size() * 3;

            for (size_t i = 0; i < primitiveIndices->size(); i++)
            {
                uint32_t indecieStatIndex = 3 * (*primitiveIndices)[i];
                _indices.push_back(indecieStatIndex);
                _indices.push_back(indecieStatIndex + 1);
                _indices.push_back(indecieStatIndex + 2);
            }

            return;
        }

        uint32_t leftChildIndex = _nodes.size();
        uint32_t rightChildIndex = _nodes.size() + 1;

        KdNode& rootNode = _nodes[nodeIndex];
        rootNode.flags = splitAxis;
        rootNode.split = splitPos;
        rootNode.startIndex = 0;
        rootNode.indeciesCount = 0;
        rootNode.left = leftChildIndex;

        _nodes.push_back({3, splitPos, 0, 0, 0});
        _nodes.push_back({3, splitPos, 0, 0, 0});

        BuildTree(leftChildIndex, leftChildBounds, allPrimitiveBounds, leftIndecies.get(), depth + 1);
        BuildTree(rightChildIndex, rightChildBounds, allPrimitiveBounds, rightIndecies.get(), depth + 1);
    }

} // namespace TracerCore