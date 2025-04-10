#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_scalar_block_layout : enable

#include "lighting.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inShadowCoord;

layout (location = 0) out vec4 outColor;

layout(buffer_reference, scalar) buffer Materials {Material m[]; };
layout(buffer_reference, scalar) buffer MatIndices {int i[]; };

float ShadowCalculation(vec4 shadowCoord, vec3 normal, vec3 lightDir)
{
    vec3 sampleCoords = shadowCoord.xyz / shadowCoord.w;
    sampleCoords.y = -sampleCoords.y;
    sampleCoords.xy = sampleCoords.xy * 0.5f + 0.5f;
    return texture(shadowMap, sampleCoords.xy).r > sampleCoords.z ? 0.0f : 1.0f;
}

void main()
{
    outColor = vec4(1, 1, 1, 1);
//    vec3 color = texture(albedoMap, vec2(inUV.x, 1.0f - inUV.y)).xyz;
//    vec3 normal = normalize(inNormal);
//    vec3 viewDir = normalize(cameraPos.xyz - inPosition.xyz);
//    float specularIntensity = texture(specularMap, vec2(inUV.x, 1.0f - inUV.y)).r;
//
//    Light mainLight;
//    mainLight.color = mainLightColor.xyz * mainLightColor.w;
//    mainLight.direction = normalize(mainLightDir.xyz);
//    mainLight.attenuation = 1.0f;
//
//    vec3 lightingColor = CalculateLighting(mainLight, material, normal, viewDir);
//    for (uint i = 0; i < min(lightCount, MAX_LIGHTS); i++)
//    {
//        Light light = GetLight(i, inPosition);
//        lightingColor += CalculateLighting(light, material, normal, viewDir);
//    }
//
//    float shadowAttenuation = ShadowCalculation(inShadowCoord, normal, mainLight.direction);
//    lightingColor = lightingColor * shadowAttenuation;
//
//    outColor = vec4(color * (ambientColor.rgb + lightingColor), 1.0f);
}