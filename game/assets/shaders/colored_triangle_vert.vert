#version 460
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

//layout(set = 0, binding = 0) uniform SceneData
//{
//    mat4 matrixM;
//    mat4 matrixV;
//    mat4 matrixP;
//    mat4 matrixVP;
//    vec4 ambientColor;
//    vec4 mainLightDir;
//    vec4 mainLightColor;
//};

struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer
{
    Vertex vertices[];
};

layout(push_constant) uniform PushConstants
{
    mat4 worldMatrix;
    VertexBuffer vertexBuffer;
};

void main()
{
    Vertex v = vertexBuffer.vertices[gl_VertexIndex];

    gl_Position = worldMatrix * vec4(v.position, 1.0f);
    outColor = vec3(1, 0, 1);
    outUV.x = v.uv_x;
    outUV.y = v.uv_y;
}