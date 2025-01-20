#define MAX_LIGHTS 8

struct Light
{
    vec3 direction;
    float attenuation;
    vec3 color;
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
