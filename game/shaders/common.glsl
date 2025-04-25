#ifndef COMMON
#define COMMON

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
};

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

#endif