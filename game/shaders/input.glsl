#define MAX_LIGHTS 8

struct LightData
{
    vec4 position;
    vec4 color;
    vec4 spotDirection;
    vec4 attenuation;
};

struct ObjectData
{
    int txtOffset;
    uint64_t vertexAddress;
    uint64_t indexAddress;
    uint64_t materialAddress;
    uint64_t matIdAddress;
};

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
    float shininess;
    float ior;
    float dissolve;
    int illum;
    int diffuseTextureId;
};

layout(push_constant) uniform PushConstants
{
    mat4 matrixM;
    mat4 matrixITM;
    uint objIndex;
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

layout(set = 0, binding = 1, scalar) readonly buffer ObjectData_
{
    ObjectData i[];
} objData;

layout(set = 0, binding = 2) uniform sampler2D[] textures;

layout(set = 0, binding = 3) uniform sampler2D shadowMap;
