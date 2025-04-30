#ifndef LIGHTING
#define LIGHTING

#include "common.glsl"

struct Light
{
    vec3 direction;
    vec3 color;
    float attenuation;
};

vec3 ComputeDiffuse(Material material, vec3 lightDir, vec3 normalWS)
{
    float NdotL = max(dot(normalWS, lightDir), 0.0f);
    vec3 diffuse = material.diffuse * NdotL;
    if (material.illum >= 1)
    {
        diffuse += material.ambient;
    }

    return diffuse;
}

vec3 ComputeSpecular(Material material, vec3 lightDir, vec3 normalWS, vec3 viewDir)
{
    if (material.illum < 2)
    {
        return vec3(0);
    }

    const float kPi = 3.14159265f;
    const float kShininess = max(material.shininess, 4.0f);

    const float kEnergyConservation = (2.0f + kShininess) / (2.0f * kPi);
    vec3 V = normalize(-viewDir);
    vec3 R = reflect(-lightDir, normalWS);
    float specular = kEnergyConservation * pow(max(dot(V, R), 0.0f), kShininess);

    return vec3(material.specular * specular);
}

vec3 CalculateLighting(Light light, Material material, vec3 normal, vec3 viewDir)
{
    vec3 attenuatedLightColor = light.color * light.attenuation;
    vec3 diffuse = ComputeDiffuse(material, light.direction, normal);
    vec3 specular = ComputeSpecular(material, light.direction, normal, viewDir);

    return diffuse * attenuatedLightColor + specular * attenuatedLightColor;
}

#endif
