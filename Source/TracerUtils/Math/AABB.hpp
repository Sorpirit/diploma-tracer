#pragma once

#include <glm/glm.hpp>

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 centroid;

    AABB() : min(glm::vec3(FLT_MAX)), max(-FLT_MAX), centroid(glm::vec3(0.0f)) {}

    AABB(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) : min(FLT_MAX), max(-FLT_MAX), centroid(glm::vec3(0.0f)) {
        min = glm::min(v1, glm::min(v2, v3));
        max = glm::max(v1, glm::max(v2, v3));
        centroid = (v1 + v2 + v3) * 0.333333f;
    }

    AABB(glm::vec3 v1, glm::vec3 v2) : min(FLT_MAX), max(-FLT_MAX), centroid(glm::vec3(0.0f)) {
        min = glm::min(v1, v2);
        max = glm::max(v1, v2);
        centroid = (v1 + v2) * 0.5f;
    }

    void Expand(const AABB& aabb) {
        min = glm::min(min, aabb.min);
        max = glm::max(max, aabb.max);
        centroid = (min + max) * 0.5f;
    }

    void Expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
        centroid = (min + max) * 0.5f;
    }

    float SurfaceArea() const {
        glm::vec3 size = max - min;
        return 2.0f * (size.x * size.y + size.x * size.z + size.y * size.z);
    }
};