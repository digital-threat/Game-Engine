#version 460
#extension GL_EXT_buffer_reference : require

#include "input.glsl"

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;

struct Vertex
{
    vec3 position;
    float uvX;
    vec3 normal;
    float uvY;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer
{
    Vertex vertices[];
};

layout(push_constant) uniform PushConstants
{
    mat4 matrixM;
    mat4 matrixITM;
    VertexBuffer vertexBuffer;
};

void main()
{
    Vertex v = vertexBuffer.vertices[gl_VertexIndex];
    gl_Position = matrixVP * matrixM * vec4(v.position, 1.0f);
    outPosition = gl_Position.xyz;
    outNormal = mat3(matrixITM) * v.normal;
    outUV.x = v.uvX;
    outUV.y = v.uvY;
}