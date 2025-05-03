#version 460
#extension GL_EXT_ray_tracing: require
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require

#include "rt_input.glsl"
#include "rt_common.glsl"

layout (location = 0) rayPayloadInEXT HitPayload inPayload;

void main()
{
    Light light;
    light.color = mainLightColor.xyz;
    light.direction = normalize(mainLightDir.xyz);

    vec3 viewDir = gl_WorldRayDirectionEXT;

    float VdotL = dot(viewDir, light.direction);
    float sun = clamp(pow(max(0, VdotL), 1), 0.0f, 1.0f);
    sun = smoothstep(0.9995f, 0.998f, sun);

    float t = clamp(viewDir.y * 0.5f + 0.5f, 0.0f, 1.0f);
    vec3 horizonColor = vec3(0.8f, 0.9f, 1.0f);
    vec3 zenithColor = vec3(0.1f, 0.3f, 0.6f);
    vec3 skyColor = mix(horizonColor, zenithColor, t);

    inPayload.color = skyColor * sun + (1 - sun) * light.color;
}