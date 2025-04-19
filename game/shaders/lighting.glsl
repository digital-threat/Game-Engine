#include "forward_input.glsl"

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

Light GetLight(uint index, vec3 position)
{
    vec3 lightColor = lightBuffer[index].color.rgb;
    float lightType = lightBuffer[index].color.w;
    vec3 lightPosition = lightBuffer[index].position.xyz;
    vec3 lightSpotDirection = lightBuffer[index].spotDirection.xyz;
    float lightSpotCutoff = lightBuffer[index].spotDirection.w;

    Light light;
    light.color = lightColor;

    if (lightType == 0.0f) // Directional
    {
        light.direction = normalize(lightPosition);
        light.attenuation = 1.0f;
    }
    else if (lightType == 1.0f) // Point
    {
        light.direction = normalize(lightPosition - position);
        float dist = length(lightPosition - position);
        light.attenuation = 1.0 / (1.0f + 0.7f * dist + 1.8f * (dist * dist));
    }
    else if (lightType == 2.0f) // Spot
    {
        light.direction = normalize(lightPosition - position);
        float SdotL = dot(lightSpotDirection, normalize(-light.direction));
        light.attenuation = SdotL > lightSpotCutoff ? 1.0f : 0.0f;
    }

    return light;
}
