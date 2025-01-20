#version 460

#include "input.glsl"
#include "lighting.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main()
{
    vec3 color = texture(albedoMap, vec2(inUV.x, 1-inUV.y)).xyz;
    vec3 normal = normalize(inNormal);
    vec3 viewDir = normalize(cameraPos - inPosition);
    float specularIntensity = texture(specularMap, vec2(inUV.x, 1-inUV.y)).r;

    vec3 lightingColor = vec3(0);
    for (uint i = 0; i < min(lightCount, MAX_LIGHTS); i++)
    {
       Light light = lights[i];
       light.attenuation = 1;
       lightingColor += CalculateLighting(light, normal, viewDir, 16.0f, specularIntensity);
    }

    outColor = vec4(color * (ambientColor + lightingColor), 1.0f);
}