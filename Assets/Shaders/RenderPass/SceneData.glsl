#ifndef SCENE_DATA_H
#define SCENE_DATA_H

layout(binding = 2, std140) uniform SceneData{
    mat4 camProjection;
    mat4 camView;
    mat4 camInvProjection;
    mat4 camInvView;
    vec3 color;
    uint frameIndex;
    uint useAccumulationTexture;
    uint accumFrameIndex;
    uint maxBounces;
    
    float defoucsDiskAngle;
    float defoucsDiskU;
    float defoucsDiskV;

    vec3 aabbMin;
    vec3 aabbMax;
} sceneData;

#endif