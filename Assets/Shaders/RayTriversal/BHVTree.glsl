#ifndef BHV_TREE_H
#define BHV_TREE_H

#include "Common.glsl"
#include "../RenderPass/VertexInput.glsl"

//Requiers BHVNodes binding 5

struct BHVNode {
    vec3 aabbMin;
    vec3 aabbMax;
    uint left;
    // uint32_t right = left + 1; as they are stored consequtevly
    uint startIndex;
    uint indeciesCount;
};

layout(binding = 5, std140) readonly buffer BHVNodes{
    BHVNode nodes[];
};

bool hitBHVTree(uint nodeIndex, ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    uint nodeQueue[32];
    nodeQueue[0] = nodeIndex;
    uint quelenght = 1;

    float bhvTMin = infinity;
    float bhvTMax = tMax;

    bool hit = false;
    HitResult currentHit;

    while(quelenght > 0)
    {
        BHVNode node = nodes[nodeQueue[quelenght - 1]];
        AABB aabb = AABB(node.aabbMin, node.aabbMax);
        quelenght--;

        if (!hitAABB(aabb, ray, bhvTMin, bhvTMax) || bhvTMin > tMax) 
        {
            continue;
        }

        if (node.indeciesCount > 0) {
            
            for(int i = 0; i < node.indeciesCount; i+= 3)
            {
                triangle tri = getTriangle(node.startIndex + i);
                if (hitTriangle(tri, ray, tMin, tMax, currentHit))
                {
                    hit = true;
                    hitResult = currentHit;
                }
            }

            continue;
        }

        nodeQueue[quelenght++] = node.left;
        nodeQueue[quelenght++] = node.left + 1;
    }

    return hit;
}

bool TreverseScene(ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    return hitBHVTree(0, ray, tMin, tMax, hitResult);
}

#endif // BHV_TREE_H