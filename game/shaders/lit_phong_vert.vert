#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : enable

#include "input.glsl"

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec4 outShadowCoord;

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
};

layout(buffer_reference, scalar) readonly buffer VertexBuffer
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
    outUV = v.uv;
    outShadowCoord = mainLightVP * matrixM * vec4(v.position, 1.0f);
}