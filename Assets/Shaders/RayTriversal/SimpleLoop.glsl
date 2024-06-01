#ifndef SIMPLE_LOOP_H
#define SIMPLE_LOOP_H

#include "Common.glsl"
#include "../RenderPass/VertexInput.glsl"

bool TreverseScene(ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    bool hit = false;
    HitResult currentHit;
    for(int i = 0; i < indecies.length(); i+= 3)
    {
        triangle tri = getTriangle(i);
        //Garanties the hit in the tMin - tMax range
        if (!hitTriangle(tri, ray, tMin, tMax, currentHit))
            continue;

        hit = true;
        hitResult = currentHit;
    }

    return hit;
}

#endif // SIMPLE_LOOP_H