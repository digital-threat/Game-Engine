#version 460
#extension GL_EXT_buffer_reference : require

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
    mat4 depthMVP;
    VertexBuffer vertexBuffer;
};

void main()
{
    Vertex v = vertexBuffer.vertices[gl_VertexIndex];
    gl_Position = depthMVP * vec4(v.position, 1.0);
}