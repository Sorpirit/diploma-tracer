#pragma once

#include <vector>
#include "TracerVertex.hpp"

namespace TracerUtils::Models
{
    struct TracerMesh
    {
        std::vector<TracerVertex> Vertices;
        std::vector<uint32_t> Indices;

        std::vector<TrianglePolygon>* GetTriangles()
        {
            std::vector<TrianglePolygon>* triangles = new std::vector<TrianglePolygon>();

            for (size_t i = 0; i < Indices.size(); i += 3)
            {
                TrianglePolygon triangle;
                triangle.V0 = glm::vec3(Vertices[Indices[i]].Position);
                triangle.V1 = glm::vec3(Vertices[Indices[i + 1]].Position);
                triangle.V2 = glm::vec3(Vertices[Indices[i + 2]].Position);
                triangle.N0 = glm::vec3(Vertices[Indices[i]].Normal);
                triangle.N1 = glm::vec3(Vertices[Indices[i + 1]].Normal);
                triangle.N2 = glm::vec3(Vertices[Indices[i + 2]].Normal);
                triangle.PrecalculatedNormals = true;
                triangles->push_back(triangle);
            }

            return triangles;
        };
    };
}