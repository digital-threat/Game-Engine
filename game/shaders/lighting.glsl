#ifndef LIGHTING
#define LIGHTING

#include "common.glsl"

struct Light
{
    vec3 direction;
    vec3 color;
    float attenuation;
};

vec3 LightingLambert(Material material, vec3 lightDir, vec3 normal)
{
    float NdotL = max(dot(normal, lightDir), 0.0f);
    vec3 color = material.diffuse * NdotL;
    if (material.illum >= 1)
    {
        color += material.ambient;
    }

    return color;
}

vec3 LightingSpecular(Material material, vec3 lightDir, vec3 normal, vec3 viewDir)
{
    if (material.illum < 2)
    {
        return vec3(0);
    }

    vec3 halfVector = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfVector), 0.0);
    vec3 color = material.specular * pow(NdotH, material.shininess);
    return color;
}

vec3 CalculateLighting(Light light, Material material, vec3 normal, vec3 viewDir)
{
    vec3 attenuatedLightColor = light.color * light.attenuation;
    vec3 diffuse = LightingLambert(material, light.direction, normal);
    vec3 specular = LightingSpecular(material, light.direction, normal, viewDir);

    return diffuse * attenuatedLightColor + specular * attenuatedLightColor;
}

#endif
