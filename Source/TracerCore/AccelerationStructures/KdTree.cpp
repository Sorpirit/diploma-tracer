#include "KdTree.hpp"
#include <algorithm>
#include "iostream"


namespace TracerCore::AccelerationStructures
{
    KdTree::KdTree(VulkanDevice &_device, AccHeruishitcType accHeruishitcType, std::vector<TracerUtils::Models::TracerVertex> &vertices, std::vector<uint32_t> &indices) 
        : AccelerationStructure(_device, vertices, indices), _accHeruishitcType(accHeruishitcType)
    {
        std::cout << "Building Kd Tree, Heruishitc " << (int) _accHeruishitcType << std::endl;

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

        KdNode root = {0, 0.5f, 0, primitiveIndices.size()};
        _nodes.push_back(root);
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

        _indeciesCount = _indices.size();
        _indices.clear();
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
        uint32_t splitAxis = -1;
        float splitPos = 0.0f;
        float bestCost = FLT_MAX;
        
        bool foundSplit = FindBestSplitPosition(_nodes[nodeIndex], nodeBounds, allPrimitiveBounds, primitiveIndices, bestCost, splitAxis, splitPos);
        if(depth > 64 || !foundSplit)
        {
            KdNode& leafNode = _nodes[nodeIndex];
            leafNode.flags = KdNodeFlags_Leaf;
            leafNode.nextIndex = _indices.size();
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
            leafNode.nextIndex = _indices.size();
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
        rootNode.nextIndex = leftChildIndex;
        rootNode.indeciesCount = 0;

        _nodes.push_back(KdNode{0, splitPos, 0, static_cast<uint32_t>(leftIndecies->size())});
        _nodes.push_back(KdNode{0, splitPos, 0, static_cast<uint32_t>(rightIndecies->size())});

        BuildTree(leftChildIndex, leftChildBounds, allPrimitiveBounds, leftIndecies.get(), depth + 1);
        if(depth == 1)
        {
            printf("Left child: %d\n", _nodes.size());
        }
        BuildTree(rightChildIndex, rightChildBounds, allPrimitiveBounds, rightIndecies.get(), depth + 1);
    }

    bool KdTree::FindBestSplitPosition(
        const KdNode &node, 
        const KdTreeBounds &nodeBounds, 
        const std::vector<KdTreeBounds> &allPrimitiveBounds, 
        const std::vector<uint32_t> *primitiveIndices, 
        float &bestCost, 
        uint32_t &bestSplitAxis, 
        float &bestSplitPos)
    {
        bestSplitAxis = 0;
        bestCost = FLT_MAX;
        bestSplitPos = 0.0f;

        switch (_accHeruishitcType)
        {
        case AccHeruishitcType::AccHeruishitc_Primitive:
        {
            if(node.indeciesCount <= 32)
            {
                return false;
            }

            glm::vec3 aabbSize = nodeBounds.aabbMax - nodeBounds.aabbMin;
            if(aabbSize.y > aabbSize.x && aabbSize.y > aabbSize.z)
            {
                bestSplitAxis = 1;
            }
            else if(aabbSize.z > aabbSize.x && aabbSize.z > aabbSize.y)
            {
                bestSplitAxis = 2;
            }

            bestSplitPos = nodeBounds.aabbMin[bestSplitAxis] + aabbSize[bestSplitAxis] * 0.5f;
            break;
        }
        case AccHeruishitcType::AccHeruishitc_SAH:
        {
            if(node.indeciesCount <= 32)
            {
                return false;
            }

            glm::vec3 aabbSize = nodeBounds.aabbMax - nodeBounds.aabbMin;
            float parentSurfaceArea = 2.0f * (aabbSize.x * aabbSize.y + aabbSize.x * aabbSize.z + aabbSize.y * aabbSize.z);
            float invParentSurfaceArea = 1.0f / parentSurfaceArea;
            float parentCost = node.indeciesCount * parentSurfaceArea;

            for (uint32_t axis = 0; axis < 3; axis++)
            {
                float boundsMin = nodeBounds.aabbMin[axis];
                float boundsMax = nodeBounds.aabbMax[axis];
                if((boundsMax - boundsMin) < 0.0001f)
                    continue;

                uint32_t splitsCount = node.indeciesCount * 2; 
                edgeVector.resize(splitsCount);
                for (uint32_t i = 0; i < node.indeciesCount; i++)
                {
                    const KdTreeBounds& bounds = allPrimitiveBounds[(*primitiveIndices)[i]];
                    edgeVector[i * 2] = {i, bounds.aabbMin[axis], false};
                    edgeVector[i * 2 + 1] = {i, bounds.aabbMax[axis], true};
                }

                std::sort(edgeVector.begin(), edgeVector.end(), [](const KdSplit& a, const KdSplit& b) { return a.tSplit < b.tSplit; });

                uint32_t primitiveLeftCount = 0;
                uint32_t primitiveRight = node.indeciesCount;
                for (uint32_t i = 0; i < splitsCount; i++)
                {
                    if(edgeVector[i].isMax)
                    {
                        primitiveRight--;
                    }

                    if(!edgeVector[i].isMax)
                    {
                        primitiveLeftCount++;
                    }
                    
                    float splitPos = edgeVector[i].tSplit;
                    if(splitPos <= boundsMin || splitPos >= boundsMax)
                    {
                        continue;
                    }

                    uint32_t axis1 = (axis + 1) % 3;
                    uint32_t axis2 = (axis + 2) % 3;
                    float leftArea = 2.0f * (aabbSize[axis1] * aabbSize[axis2] + (splitPos - boundsMin) * (aabbSize[axis1] + aabbSize[axis2]));
                    float rightArea = 2.0f * (aabbSize[axis1] * aabbSize[axis2] + (boundsMax - splitPos) * (aabbSize[axis1] + aabbSize[axis2]));
                    float cost = _traversalCost + leftArea * primitiveLeftCount + rightArea * primitiveRight;

                    if(cost < bestCost)
                    {
                        bestCost = cost;
                        bestSplitAxis = axis;
                        bestSplitPos = splitPos;
                    }
                }
            }

            if(bestCost < 1.0f || bestCost >= parentCost)
            {
                return false;
            }
            break;
        }
        default:
            break;
        }

        return true;
    }

    float KdTree::CalculateSAHNodeCost(const KdNode &node, const KdTreeBounds &nodeBounds)
    {
        glm::vec3 aabbSize = nodeBounds.aabbMax - nodeBounds.aabbMin;
        float parentSurfaceArea = 2.0f * (aabbSize.x * aabbSize.y + aabbSize.x * aabbSize.z + aabbSize.y * aabbSize.z);
        return (node.indeciesCount) * parentSurfaceArea;
    }

} // namespace TracerCore