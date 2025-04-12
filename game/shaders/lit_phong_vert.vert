#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
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

layout(buffer_reference, scalar) buffer VertexBuffer
{
    Vertex vertices[];
};

void main()
{
    ObjectData data = objData.i[objIndex];
    VertexBuffer vertexBuffer = VertexBuffer(data.vertexAddress);
    Vertex v = vertexBuffer.vertices[gl_VertexIndex];
    gl_Position = matrixVP * matrixM * vec4(v.position, 1.0f);
    outPosition = gl_Position.xyz;
    outNormal = mat3(matrixITM) * v.normal;
    outUV = v.uv;
    outShadowCoord = mainLightVP * matrixM * vec4(v.position, 1.0f);
}