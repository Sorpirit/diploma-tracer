#pragma once

#include "glm/glm.hpp"

namespace TracerUtils::Models
{
    struct TracerVertex
    {
        alignas(16) glm::vec3 Position;
        alignas(16) glm::vec3 Normal;
        alignas(8) glm::vec2 TextureCoordinate;
        alignas(4) uint32_t MaterialFlag = 1;
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

        alignas(16) glm::vec3 Center;
    };

}

