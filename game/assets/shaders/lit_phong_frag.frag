#version 460

#include "input.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;

void main()
{
    vec3 color = texture(albedoMap, inUV).xyz;
    vec3 normal = normalize(inNormal);
    float NdotL = max(dot(normal, mainLightDir), 0.0f);

    vec3 ambient = ambientColor;
    vec3 diffuse = NdotL * mainLightColor;

    vec3 viewDir = normalize(cameraPos - inPosition);
    vec3 halfVector = normalize(mainLightDir + viewDir);
    float brightness = max(dot(normal, halfVector), 0.0);
    vec3 specular = pow(brightness, 16.0f) * mainLightColor;

    outColor = vec4(color * (ambient + diffuse + specular), 1.0f);
}