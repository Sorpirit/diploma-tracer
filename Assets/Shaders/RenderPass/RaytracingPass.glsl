#include "../Utils/random.glsl"
#include "../Utils/constants.glsl"
#include "../Utils/color.glsl"
#include "VertexInput.glsl"
#include "SceneData.glsl"

#if defined(USE_BVH)
#include "../RayTriversal/BHVTree.glsl"
#elif defined(USE_KD_TREE)
#include "../RayTriversal/KdTree.glsl"
#elif defined(USE_LOOP)
#include "../RayTriversal/SimpleLoop.glsl"
#endif

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

//Ray tracing results
layout(binding = 0, rgba8) uniform writeonly image2D resultImage;
layout(binding = 1, rgba32f) uniform image2D accumulationTexture;



vec3 ray_refract(vec3 rayDirection, vec3 surfaceNormal, float etai_over_etat) {
    float cos_theta = min(dot(-rayDirection, surfaceNormal), 1.0);
    vec3 r_out_parallel = etai_over_etat * (rayDirection + cos_theta * surfaceNormal);
    vec3 r_out_perp = -sqrt(abs(1.0 - dot(r_out_parallel, r_out_parallel))) * surfaceNormal;
    return r_out_parallel + r_out_perp;
}

float reflectance(float cosine, float refraction_index) {
    // Use Schlick's approximation for reflectance.
    float r0 = (1.0 - refraction_index) / (1.0 + refraction_index);
    r0 = r0*r0;
    return r0 + (1.0-r0)*pow((1.0 - cosine),5);
}

void Scater(HitResult hit, inout ray ray, inout vec3 attenuation)
{
    switch(hit.materialFlag)
    {
        //lambertian
        case 0:
            attenuation *= vec3(0.8, 0.8, 0.0); //albedo
            ray.origin = hit.hitPoint + hit.normal * 0.01;
            ray.direction = hit.normal + randomUnitVector();
            if(dot(ray.direction, hit.normal) < 0.0001) 
                ray.direction = hit.normal;
            break;

        //metal            
        case 1:
            float fuzz = 0.01;
            attenuation *= vec3(0.8, 0.8, 0.0); //albedo
            ray.origin = hit.hitPoint + hit.normal * 0.01;
            ray.direction = normalize(reflect(ray.direction, hit.normal)) + fuzz * randomUnitVector();
            break;

        //glass
        case 2:
            float refractionIndex = 1.0 / 1.33;
            //attenuation not changed
            ray.origin = hit.hitPoint;

            float ri = hit.frontFace ? 1.0 / refractionIndex : refractionIndex;
            vec3 unitDir = normalize(ray.direction);
            float cosTheta = min(dot(-unitDir, hit.normal), 1.0);
            float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

            bool cannotRefract = ri * sinTheta > 1.0;
            vec3 direction = cannotRefract || reflectance(cosTheta, ri) > hash1() ? reflect(unitDir, hit.normal) : ray_refract(unitDir, hit.normal, ri);
            ray.direction = direction;
        break;
    }
}
vec3 GetRayDirection(vec2 screenCord)
{
    vec4 target = sceneData.camInvProjection * vec4(screenCord, 1.0, 1.0);
    vec3 dir = vec3(sceneData.camInvView * vec4(normalize(vec3(target.xyz / target.w)), 0.0));
    return normalize(dir);
}

vec3 TraceRay(ray ray)
{
    sphere spheres[1];

    spheres[0] = sphere(vec3(0,-100.5, -1), 100, 0);
    // spheres[1] = sphere(vec3(1.8, 0.0, -0.1), 0.5, 0);
    // spheres[2] = sphere(vec3(-1.8, 0.0, -0.1), 0.5, 2);
    
    
    uint maxBounces = sceneData.maxBounces;

    vec3 finalColor = vec3(0);
    vec3 color = vec3(1);
        
    for(int d = 0; d < maxBounces; d++) 
    {
        const float tMin = 0.001;
        float tMax = infinity;

        HitResult currentHit;
        HitResult closestHit;
        bool hit = false;

        for(int i = 0; i < spheres.length(); i++)
        {
            //Garanties the hit in the tMin - tMax range
            if (!hitSphere(spheres[i], ray, tMin, tMax, currentHit))
                continue;

            hit = true;
            closestHit = currentHit;
        }

        if(TreverseScene(ray, tMin, tMax, currentHit))
        {
            hit = true;
            closestHit = currentHit;
        }

        if (hit) 
        {
            Scater(closestHit, ray, color);
        }
        else 
        {
            vec3 unitDir = normalize(ray.direction);
            float a = 0.5 * (unitDir.y + 1.0);
            vec3 skyColor =  (1.0 - a * vec3(1) + a * sceneData.color);
            finalColor = color * skyColor;
            break;
        }        
    }
    return finalColor;
}


void main() 
{
    vec2 imageSize = vec2(imageSize(resultImage));
    ivec2 textureCoord = ivec2(gl_GlobalInvocationID.xy);
    
    //calculate uv coords
    vec2 uv = textureCoord / imageSize.xy;
    vec2 screenCord = uv * 2 - 1;
    screenCord.y *= -1;
    
    // modify random seed
    gState *= floatBitsToUint(uv.y * uv.x / 1000) * textureCoord.x * sceneData.frameIndex;

    vec3 origin = sceneData.camInvView[3].xyz;

    //anti aliasing
    uint samplesPerPixel = 4;
    float influence = 0.0025;
    vec3 frameColor = vec3(0);    
    for(int i = 0; i < samplesPerPixel; i++) {
        vec3 dir = GetRayDirection(screenCord) + influence * hash3();
        vec3 invDir = 1.0 / dir;
        ray ray = ray(origin, dir, invDir);
        frameColor += TraceRay(ray);
    }
    frameColor /= samplesPerPixel;

    vec3 color = frameColor.rgb;
    int useAcc = int(sceneData.useAccumulationTexture);
    int firstAcc = int(sceneData.accumFrameIndex != 0);

    vec3 currentColor = frameColor + useAcc * imageLoad(accumulationTexture, ivec2(gl_GlobalInvocationID.xy)).rgb;

    if(firstAcc == 1)
    {
        imageStore(accumulationTexture, ivec2(gl_GlobalInvocationID.xy), vec4(currentColor, 1));
    }
        
    color = currentColor.rgb / (useAcc * sceneData.accumFrameIndex + 1);
    color = gamma_correction(color);
    color = clamp(color, 0, 1);

    imageStore(resultImage, textureCoord, vec4(color.rgb, 1));
}