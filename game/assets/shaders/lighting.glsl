#include "input.glsl"

struct Light
{
    vec3 direction;
    vec3 color;
    float attenuation;
};

vec3 LightingLambert(vec3 lightColor, vec3 lightDir, vec3 normal)
{
    float NdotL = max(dot(normal, lightDir), 0.0f);
    return lightColor * NdotL;
}

vec3 LightingSpecular(vec3 lightColor, vec3 lightDir, vec3 normal, vec3 viewDir, float smoothness, float intensity)
{
    vec3 halfVector = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfVector), 0.0);
    return lightColor * pow(NdotH, smoothness) * intensity;
}

vec3 CalculateLighting(Light light, vec3 normal, vec3 viewDir, float smoothness, float intensity)
{
    vec3 attenuatedLightColor = light.color * light.attenuation;
    vec3 diffuse = LightingLambert(attenuatedLightColor, light.direction, normal);
    vec3 specular = LightingSpecular(attenuatedLightColor, light.direction, normal, viewDir, smoothness, intensity);

    return diffuse + specular;
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
