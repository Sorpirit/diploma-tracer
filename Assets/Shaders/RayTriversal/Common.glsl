#ifndef COMMON_H
#define COMMON_H

#include "../RenderPass/VertexInput.glsl"

struct AABB {
    vec3 aabbMin;
    vec3 aabbMax;
};

struct ray {
    vec3 origin;
    vec3 direction;
    vec3 invDirection;
};

struct HitResult {
    vec3 hitPoint;
    vec3 normal;
    float hitDistance;
    bool frontFace;
    uint materialFlag;
};

struct sphere {
    vec3 center;
    float radius;
    uint materialFlag;
};

bool hitSphere(sphere sphere, ray ray, float tMin, inout float tMax, inout HitResult hitResult)
{
    //solve sphere equation
    vec3 origin = sphere.center - ray.origin;
    
    float a = dot(ray.direction, ray.direction);
    float h = dot(origin, ray.direction);
    float c = dot(origin, origin) - sphere.radius * sphere.radius;

    float discriminant = h * h - a * c;
    if(discriminant < 0) return false;
    
    float sqrtDiscriminant = sqrt(discriminant);

    //calculate hitpoint and normal
    float t0 = (h - sqrtDiscriminant / a);
    if(t0 < tMin || t0 > tMax) 
    {
        t0 = (h + sqrtDiscriminant / a);
        if(t0 < tMin || t0 > tMax) return false;
    }

    //Collision happened
    tMax = t0;

    vec3 hitPoint = ray.origin + t0 * ray.direction;
    vec3 normal = normalize(hitPoint - sphere.center);

    hitResult.hitDistance = t0;
    hitResult.hitPoint = hitPoint;
    hitResult.frontFace = dot(ray.direction, normal) < 0;
    hitResult.normal = hitResult.frontFace ? normal : -normal;
    hitResult.materialFlag = sphere.materialFlag;

    return true;
}

bool hitTriangle(triangle tri, ray ray, float tMin, inout float tMax, inout HitResult hitResult) 
{
    vec3 v0v1 = tri.v1 - tri.v0;
    vec3 v0v2 = tri.v2 - tri.v0;
    vec3 pVec = cross(ray.direction, v0v2);
    float det = dot(v0v1, pVec);
    
    if(abs(det) < epsilon) 
    {
        return false;
    } 
    
    float invDet = 1 / det;
    vec3 tVec = ray.origin - tri.v0;
    float u = dot(tVec, pVec) * invDet;
    vec3 qVec = cross(tVec, v0v1);
    float v = dot(ray.direction, qVec) * invDet;
    
    if(u < 0.0 || v < 0.0 || (u + v) > 1.0) return false;

    float t = dot(v0v2, qVec) * invDet;
    if (t < tMin || t > tMax) return false;
    
    tMax = t;
    hitResult.hitDistance = t;
    hitResult.hitPoint = ray.origin + t * ray.direction;
    hitResult.normal = vec3(u,v,1-u-v);
    hitResult.normal = tri.n1 * u + tri.n2 * v + tri.n0 * (1 - u - v);
    
    if(det < 0) 
        hitResult.normal *= -1;

    hitResult.frontFace = dot(ray.direction, hitResult.normal) < 0;
    hitResult.materialFlag = tri.materialFlag;
    
    return true;
}

bool hitAABB(AABB aabb, ray ray, inout float tStart, inout float tEnd) 
{
    // if(ray.direction.x == 0 && (ray.origin.x < aabb.aabbMin.x || ray.origin.x > aabb.aabbMax.x)) return false;
    // if(ray.direction.y == 0 && (ray.origin.y < aabb.aabbMin.y || ray.origin.y > aabb.aabbMax.y)) return false;
    // if(ray.direction.z == 0 && (ray.origin.z < aabb.aabbMin.z || ray.origin.z > aabb.aabbMax.z)) return false;

    vec3 t0 = (aabb.aabbMin - ray.origin) * ray.invDirection;
    vec3 t1 = (aabb.aabbMax - ray.origin) * ray.invDirection;
    vec3 tminv = min(t0, t1);
    vec3 tmaxv = max(t0, t1);

    tStart = max(tminv.x, max(tminv.y, tminv.z));
    tEnd = min(tmaxv.x, min(tmaxv.y, tmaxv.z));

    return tEnd >= tStart && tEnd > 0;
}

bool hitAABB(AABB aabb, ray ray)
{
    float tStart = 0;
    float tEnd = 0;
    return hitAABB(aabb, ray, tStart, tEnd);
}

#endif //COMMON_H