#include "lighting.glsl"

layout(set = 0, binding = 0) uniform Scene
{
    mat4 matrixV;
    mat4 matrixP;
    mat4 matrixVP;
    vec3 ambientColor;
    vec3 mainLightDir;
    vec3 mainLightColor;
    vec3 cameraPos;
    Light lights[MAX_LIGHTS];
    uint lightCount;
};

//layout(set = 1, binding = 0) uniform Material
//{
//    float shininess;
//};

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D specularMap;