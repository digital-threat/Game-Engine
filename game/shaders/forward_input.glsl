#define MAX_LIGHTS 8

#include "common.glsl"

layout (push_constant) uniform PushConstants
{
    mat4 matrixM;
    mat4 matrixITM;
    uint objIndex;
};

layout (set = 0, binding = 0) uniform Scene
{
    mat4 matrixV;
    mat4 matrixP;
    mat4 matrixIV;
    mat4 matrixIP;
    mat4 matrixVP;
    mat4 mainLightVP;
    vec4 mainLightDir;
    vec4 mainLightColor;
    vec4 ambientColor;
    vec4 cameraPos;
    LightData lightBuffer[MAX_LIGHTS];
    uint lightCount;
};

layout (set = 0, binding = 1, scalar) readonly buffer ObjectData_
{
    ObjectData i[];
} objData;

layout (set = 0, binding = 2) uniform sampler2D[] textures;

layout (set = 0, binding = 3) uniform sampler2D shadowMap;
