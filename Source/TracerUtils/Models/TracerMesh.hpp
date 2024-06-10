#pragma once

#include <vector>
#include "TracerVertex.hpp"

namespace TracerUtils::Models
{
    struct TracerMeshPart
    {
        std::vector<TracerVertex> Vertices;
        std::vector<uint32_t> Indices;
    };

    struct TracerMesh
    {
        std::vector<TracerMeshPart> Parts;
    };
}