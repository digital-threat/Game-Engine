#define MAX_LIGHTS 8

struct LightData
{
    vec4 position;
    vec4 color;
    vec4 spotDirection;
    vec4 attenuation;
};

struct Material
{
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    vec3  transmittance;
    vec3  emission;
    float shininess;
    float ior;
    float dissolve;
    int   illum;
    int   textureId;
};

layout(set = 0, binding = 0) uniform Scene
{
    mat4 matrixV;
    mat4 matrixP;
    mat4 matrixVP;
    mat4 mainLightVP;
    vec4 mainLightDir;
    vec4 mainLightColor;
    vec4 ambientColor;
    vec4 cameraPos;
    LightData lightBuffer[MAX_LIGHTS];
    uint lightCount;
};

layout(set = 0, binding = 1) uniform sampler2D shadowMap;

//layout(set = 1, binding = 0) uniform Material
//{
//    float shininess;
//};

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D specularMap;
