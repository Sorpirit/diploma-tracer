#ifndef KD_TREE_H
#define KD_TREE_H

#include "Common.glsl"
#include "../RenderPass/VertexInput.glsl"
#include "../RenderPass/SceneData.glsl"

//Requiers KdNodes binding 5

struct KdNode {
    uint flags;
    float split;
    uint left;
    // uint32_t right = left + 1; as they are stored consequtevly
    uint startIndex;
    uint indeciesCount;
};

layout(binding = 5) readonly buffer KdNodes{
    KdNode nodes[];
};

bool hitKdTree(uint nodeIndex, ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    uint nodeQueue[32];
    nodeQueue[0] = nodeIndex;
    uint quelenght = 1;

    float tStart = tMin;
    float tEnd = tMax;

    AABB rootAABB = AABB(sceneData.aabbMin, sceneData.aabbMax);
    if(!hitAABB(rootAABB, ray, tStart, tEnd) || tStart > tMax) 
        return false;

    bool hit = false;
    HitResult currentHit;

    while(quelenght > 0)
    {
        KdNode node = nodes[nodeQueue[quelenght - 1]];
        quelenght--;

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

            //early exit
            if(hit)
                return true;

            continue;
        }

        
        float t = (node.split - ray.origin[node.flags]) / ray.direction[node.flags];
        
        bool leftToRightOrder = (ray.origin[node.flags] < node.split) || (ray.origin[node.flags] == node.split && ray.direction[node.flags] <= 0);
        
        uint nearChild = leftToRightOrder ? node.left : node.left + 1;
        uint farChild = leftToRightOrder ? node.left + 1 : node.left;

        if(t > tEnd || t <= 0)
        {
            nodeQueue[quelenght++] = nearChild;
        }
        else if(t < tStart)
        {
            nodeQueue[quelenght++] = farChild;
        }
        else
        {
            nodeQueue[quelenght++] = farChild;
            nodeQueue[quelenght++] = nearChild;
        }
    }

    return hit;
}

bool TreverseScene(ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    return hitKdTree(0, ray, tMin, tMax, hitResult);
}

#endif //KD_TREE_H