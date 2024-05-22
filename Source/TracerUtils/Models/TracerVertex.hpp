#pragma once

#include "glm/glm.hpp"

namespace TracerUtils::Models
{
    struct TracerVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TextureCoordinate;
    };

    struct TrianglePolygon
    {
        alignas(16) glm::vec3 V0;
        alignas(16) glm::vec3 V1;
        alignas(16) glm::vec3 V2;
        alignas(16) glm::vec3 N0;
        alignas(16) glm::vec3 N1;
        alignas(16) glm::vec3 N2;
        
        alignas(4) bool PrecalculatedNormals = 0;
        alignas(4) uint32_t MaterialFlag = 1;
    };

}

