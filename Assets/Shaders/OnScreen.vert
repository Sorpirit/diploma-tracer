#version 450

layout(location = 0) out vec2 outUV;

vec2 position[6] = vec2[](
    vec2(-1.0, -1.0), 
    vec2(1.0, -1.0), 
    vec2(-1.0, 1.0),

    vec2(-1.0, 1.0),
    vec2(1.0, -1.0), 
    vec2(1.0, 1.0)
);

vec2 uv[6] = vec2[](
    vec2(0.0, 0.0), 
    vec2(1.0, 0.0), 
    vec2(0.0, 1.0), 

    vec2(0.0, 1.0),
    vec2(1.0, 0.0), 
    vec2(1.0, 1.0)
);

void main()
{
    gl_Position = vec4(position[gl_VertexIndex], 0.0, 1.0);
    outUV = uv[gl_VertexIndex];
}