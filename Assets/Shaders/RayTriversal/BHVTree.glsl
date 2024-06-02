#ifndef BHV_TREE_H
#define BHV_TREE_H

#include "Common.glsl"
#include "../RenderPass/VertexInput.glsl"

//Requiers BHVNodes binding 5

struct BHVNode {
    vec3 aabbMin;
    vec3 aabbMax;

    // if indeciesCount > 0 then this is a leaf node so nextIndex means offset in the indecies buffer. If indeciesCount == 0 then this is a branch node so nextIndex means offset in the nodes buffer
    uint nextIndex;
    uint indeciesCount;
};

layout(binding = 5, std140) readonly buffer BHVNodes{
    BHVNode nodes[];
};

bool hitBHVTree(uint nodeIndex, ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    float bhvTMin = infinity;
    float bhvTMax = tMax;

    bool hit = false;
    HitResult currentHit;

    BHVNode root = nodes[nodeIndex];
    AABB rootAABB = AABB(root.aabbMin, root.aabbMax);
    if (!hitAABB(rootAABB, ray, bhvTMin, bhvTMax) || bhvTMin > tMax) 
    {
        return false;
    }

    uint nodeQueue[64];
    nodeQueue[0] = nodeIndex;
    uint quelenght = 1;

    while(quelenght > 0)
    {
        root = nodes[nodeQueue[quelenght - 1]];
        rootAABB = AABB(root.aabbMin, root.aabbMax);
        quelenght--;

        if (!hitAABB(rootAABB, ray, bhvTMin, bhvTMax) || bhvTMin > tMax) 
        {
            continue;
        }

        if (root.indeciesCount > 0) {
            
            for(int i = 0; i < root.indeciesCount; i+= 3)
            {
                triangle tri = getTriangle(root.nextIndex + i);
                if (hitTriangle(tri, ray, tMin, tMax, currentHit))
                {
                    hit = true;
                    hitResult = currentHit;
                }
            }

            continue;
        }

        // Primtive queing
        // nodeQueue[quelenght++] = root.nextIndex;
        // nodeQueue[quelenght++] = root.nextIndex + 1;

        // Aproximate closest primitive queing
        uint childLeftIndex = root.nextIndex;
        uint childRightIndex = root.nextIndex + 1;
        vec3 aabbLeftDist = nodes[childLeftIndex].aabbMin - ray.origin;
        vec3 aabbRightDist = nodes[childRightIndex].aabbMin - ray.origin;
        float sqrDstLeft = dot(aabbLeftDist, aabbLeftDist);
        float sqrDstRight = dot(aabbRightDist, aabbRightDist);
        if(sqrDstLeft < sqrDstRight)
        {
            nodeQueue[quelenght++] = childRightIndex;
            nodeQueue[quelenght++] = childLeftIndex;
        }
        else
        {
            nodeQueue[quelenght++] = childLeftIndex;
            nodeQueue[quelenght++] = childRightIndex;
        }
    }

    return hit;
}

bool TreverseScene(ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    return hitBHVTree(0, ray, tMin, tMax, hitResult);
}

#endif // BHV_TREE_H