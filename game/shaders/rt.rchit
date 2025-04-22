#version 460
#extension GL_EXT_ray_tracing: require
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require
#extension GL_EXT_buffer_reference2: require

#include "rt_input.glsl"
#include "rt_common.glsl"

layout (location = 0) rayPayloadInEXT HitPayload inPayload;

hitAttributeEXT vec3 attributes;

layout (buffer_reference, scalar) buffer VertexBuffer
{
    Vertex vertices[];
};

layout (buffer_reference, scalar) buffer IndexBuffer
{
    ivec3 indices[];
};

layout (buffer_reference, scalar) buffer MaterialBuffer
{
    Material materials[];
};

layout (buffer_reference, scalar) buffer MatIdBuffer
{
    int indices[];
};

void main()
{
    ObjectData data = objData.i[gl_InstanceCustomIndexEXT];
    VertexBuffer vertexBuffer = VertexBuffer(data.vertexAddress);
    IndexBuffer indexBuffer = IndexBuffer(data.indexAddress);
    MaterialBuffer materialBuffer = MaterialBuffer(data.materialAddress);
    MatIdBuffer matIdBuffer = MatIdBuffer(data.matIdAddress);

    ivec3 primitive = indexBuffer.indices[gl_PrimitiveID];

    Vertex v0 = vertexBuffer.vertices[primitive.x];
    Vertex v1 = vertexBuffer.vertices[primitive.y];
    Vertex v2 = vertexBuffer.vertices[primitive.z];

    const vec3 barycentrics = vec3(1.0 - attributes.x - attributes.y, attributes.x, attributes.y);

    const vec3 position = v0.position * barycentrics.x + v1.position * barycentrics.y + v2.position * barycentrics.z;
    const vec3 positionWS = vec3(gl_ObjectToWorldEXT * vec4(position, 1.0));

    const vec3 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
    const vec3 normalWS = normalize(vec3(normal * gl_WorldToObjectEXT));

    int matId = matIdBuffer.indices[gl_PrimitiveID];
    Material material = materialBuffer.materials[matId];

    vec3 color = vec3(1.0f, 0.0f, 1.0f);

    if (material.diffuseTextureId >= 0)
    {
        uint index = data.txtOffset + material.diffuseTextureId;
        vec2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;
        color = texture(textures[nonuniformEXT(index)], uv).xyz;
    }

    Light mainLight;
    mainLight.color = mainLightColor.xyz;
    mainLight.direction = normalize(mainLightDir.xyz);
    mainLight.attenuation = 1.0f;

    vec3 lightingColor = LightingLambert(material, mainLight.direction, normal);
    lightingColor = clamp(lightingColor, 0.0f, 1.0f);

    inPayload.hitValue = color * lightingColor;
}
