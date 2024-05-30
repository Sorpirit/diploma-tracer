#include "KdTree.hpp"

namespace TracerCore
{
    KdTree::KdTree(VulkanDevice &_device, std::vector<TracerUtils::Models::TracerVertex> &vertices, std::vector<uint32_t> &indices) : _device(_device)
    {
        //Compute bounds
        std::vector<KdTreeBounds> bounds;
        for (size_t i = 0; i < indices.size(); i+=3)
        {
            KdTreeBounds triangleBounds;
            triangleBounds.AddVertex(vertices[indices[i]].Position);
            triangleBounds.AddVertex(vertices[indices[i + 1]].Position);
            triangleBounds.AddVertex(vertices[indices[i + 2]].Position);

            bounds.push_back(triangleBounds);
            _rootBounds.AddBounds(triangleBounds);
        }
        

        BuildTree(0, _rootBounds, vertices, indices, indices.size() / 3, 1);
    }

    KdTree::~KdTree()
    {
    }

    void KdTree::BuildTree(
        int nodeIndex, 
        const KdTreeBounds &bounds, 
        std::vector<TracerUtils::Models::TracerVertex> &vertices, 
        std::vector<uint32_t> &indices,
        int primitivesCount,
        uint32_t depth)
    {
        if(primitivesCount <= 2 || depth > 64)
        {
            KdNode& leafNode = _nodes[nodeIndex];
            leafNode.flags = 3;
            leafNode.startIndex = indices.size();
            leafNode.indeciesCount = primitivesCount * 3;
            return;
        }

        glm::vec3 aabbSize = bounds.aabbMax - bounds.aabbMin;
        uint32_t splitAxis = 0;
        if(aabbSize.y > aabbSize.x && aabbSize.y > aabbSize.z)
        {
            splitAxis = 1;
        }
        else if(aabbSize.z > aabbSize.x && aabbSize.z > aabbSize.y)
        {
            splitAxis = 2;
        }

        float splitPos = bounds.aabbMin[splitAxis] + aabbSize[splitAxis] * 0.5f;

        int leftPrimitivesCount = 0;
        int rightPrimitivesCount = 0;

        KdTreeBounds leftChild = bounds;
        KdTreeBounds rightChild = bounds;
        leftChild.aabbMax[splitAxis] = splitPos;
        rightChild.aabbMin[splitAxis] = splitPos;

        KdNode& rootNode = _nodes[nodeIndex];
        rootNode.flags = splitAxis;
        rootNode.split = splitPos;
        rootNode.startIndex = 0;
        rootNode.indeciesCount = 0;
        rootNode.left = _nodes.size();

        _nodes.push_back({0, splitPos, 0, 0, 0});
        _nodes.push_back({0, splitPos, 0, 0, 0});
        BuildTree(rootNode.left, leftChild, vertices, indices, leftPrimitivesCount, depth + 1);
        BuildTree(rootNode.left + 1, rightChild, vertices, indices, rightPrimitivesCount, depth + 1);
    }

} // namespace TracerCore