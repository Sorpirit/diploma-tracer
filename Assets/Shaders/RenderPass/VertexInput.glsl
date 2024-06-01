#ifndef VERTEX_INPUT
#define VERTEX_INPUT

struct triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3 n0;
    vec3 n1;
    vec3 n2;
    uint materialFlag;
};

struct vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;
    uint materialFlag;
};

layout(binding = 3, std140) readonly buffer Triangles{
    vertex vertecies[];
};

layout(binding = 4) readonly buffer Indecies{
    uint indecies[];
};

triangle getTriangle(uint startIndex) {
    return triangle(
        vertecies[indecies[startIndex]].position,
        vertecies[indecies[startIndex + 1]].position,
        vertecies[indecies[startIndex + 2]].position,
        vertecies[indecies[startIndex]].normal,
        vertecies[indecies[startIndex + 1]].normal,
        vertecies[indecies[startIndex + 2]].normal,
        vertecies[indecies[startIndex]].materialFlag
    );
}

#endif