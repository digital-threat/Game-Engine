#version 460
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_nonuniform_qualifier: enable

#include "lighting.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inShadowCoord;

layout (location = 0) out vec4 outColor;

layout (buffer_reference, scalar) buffer Materials
{
    Material m[];
};

layout (buffer_reference, scalar) buffer MatIds
{
    int i[];
};

float ShadowCalculation(vec4 shadowCoord, vec3 normal, vec3 lightDir)
{
    vec3 sampleCoords = shadowCoord.xyz / shadowCoord.w;
    sampleCoords.y = -sampleCoords.y;
    sampleCoords.xy = sampleCoords.xy * 0.5f + 0.5f;
    return texture(shadowMap, sampleCoords.xy).r > sampleCoords.z ? 0.0f : 1.0f;
}

void main()
{
    ObjectData data = objData.i[objIndex];
    MatIds matIds = MatIds(data.matIdAddress);
    Materials materials = Materials(data.materialAddress);

    int matId = matIds.i[gl_PrimitiveID];
    Material material = materials.m[matId];

    vec3 color = vec3(1.0f, 0.0f, 1.0f);

    if (material.diffuseTextureId >= 0)
    {
        int txtOffset = data.txtOffset;
        uint txtId = txtOffset + material.diffuseTextureId;
        vec3 diffuseTxt = texture(textures[nonuniformEXT(txtId)], vec2(inUV.x, 1.0f - inUV.y)).xyz;
        color = diffuseTxt;
    }

    vec3 normal = normalize(inNormal);
    vec3 viewDir = normalize(cameraPos.xyz - inPosition.xyz);

    Light mainLight;
    mainLight.color = mainLightColor.xyz * mainLightColor.w;
    mainLight.direction = normalize(mainLightDir.xyz);
    mainLight.attenuation = 1.0f;

    vec3 lightingColor = CalculateLighting(mainLight, material, normal, viewDir);
    for (uint i = 0; i < min(lightCount, MAX_LIGHTS); i++)
    {
        Light light = GetLight(i, inPosition);
        lightingColor += CalculateLighting(light, material, normal, viewDir);
    }

    float shadowAttenuation = ShadowCalculation(inShadowCoord, normal, mainLight.direction);
    lightingColor = lightingColor * shadowAttenuation;

    outColor = vec4(color * lightingColor, 1.0f);
}