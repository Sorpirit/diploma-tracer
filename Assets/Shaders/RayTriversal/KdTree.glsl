#ifndef KD_TREE_H
#define KD_TREE_H

#include "Common.glsl"
#include "../RenderPass/VertexInput.glsl"
#include "../RenderPass/SceneData.glsl"

//Requiers KdNodes binding 5

struct KdNode {
    uint flags;
    float split;
    // if indeciesCount > 0 then this is a leaf node so nextIndex means offset in the indecies buffer. If indeciesCount == 0 then this is a branch node so nextIndex means offset in the nodes buffer
    uint nextIndex;
    uint indeciesCount;
};

struct KdNodeProcess
{
    uint nodeIndex;
    float tStart;
    float tEnd;
};

layout(binding = 5) readonly buffer KdNodes{
    KdNode nodes[];
};

bool hitKdTree(uint nodeIndex, ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    float tStart = tMin;
    float tEnd = tMax;

    AABB rootAABB = AABB(sceneData.aabbMin, sceneData.aabbMax);
    if(!hitAABB(rootAABB, ray, tStart, tEnd)) 
        return false;

    if(tStart > tMax)
        return false;

    KdNodeProcess nodeQueue[64];
    nodeQueue[0] = KdNodeProcess(nodeIndex, tStart, tEnd);
    uint quelenght = 1;

    bool hit = false;
    HitResult currentHit;

    while(quelenght > 0)
    {
        KdNodeProcess currentProcess = nodeQueue[quelenght - 1];
        KdNode node = nodes[currentProcess.nodeIndex];
        tStart = currentProcess.tStart;
        tEnd = currentProcess.tEnd;
        quelenght--;

        if(tStart > tMax)
            break;

        if (node.indeciesCount > 0) {
            for(int i = 0; i < node.indeciesCount; i+= 3)
            {
                triangle tri = getTriangle(node.nextIndex + i);
                if (hitTriangle(tri, ray, tMin, tMax, currentHit))
                {
                    hit = true;
                    hitResult = currentHit;
                }
            }

            continue;
        }

        
        float t = (node.split - ray.origin[node.flags]) * ray.invDirection[node.flags];
        
        bool leftToRightOrder = (ray.origin[node.flags] < node.split) || (ray.origin[node.flags] == node.split && ray.direction[node.flags] <= 0);
        
        uint nearChild = leftToRightOrder ? node.nextIndex : node.nextIndex + 1;
        uint farChild = leftToRightOrder ? node.nextIndex + 1 : node.nextIndex;

        if(t > tEnd || t <= 0)
        {
            nodeQueue[quelenght++] = KdNodeProcess(nearChild, tStart, tEnd);
        }
        else if(t < tStart)
        {
            nodeQueue[quelenght++] = KdNodeProcess(farChild, tStart, tEnd);
        }
        else
        {
            nodeQueue[quelenght++] = KdNodeProcess(farChild, t, tEnd);
            nodeQueue[quelenght++] = KdNodeProcess(nearChild, tStart, t);
        }
    }

    return hit;
}

bool TreverseScene(ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    return hitKdTree(0, ray, tMin, tMax, hitResult);
}

#endif //KD_TREE_H