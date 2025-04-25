#version 460
#extension GL_EXT_ray_tracing: require

#include "rt_common.glsl"

layout (push_constant) uniform PushConstants
{
    vec4 clearColor;
};

layout (location = 0) rayPayloadInEXT HitPayload inPayload;

void main()
{
    inPayload.color = clearColor.xyz;
}