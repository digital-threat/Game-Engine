#pragma once

#include <render_object.h>
#include <glm/glm.hpp>
#include <lights.h>

#include <array>
#include <vector>
#include <vk_raytracing.h>

struct SceneRenderData
{
	glm::vec3 mainLightPos;
	glm::vec4 mainLightColor;
	glm::vec3 ambientColor;
};

struct CameraRenderData
{
	glm::vec3 pos;
	glm::vec3 forward;
	glm::vec3 up;
	float fov;
};

struct LightRenderData
{
	std::array<LightData, MAX_LIGHTS> lightBuffer;
	u32 lightCount;
};

struct RaytracingData
{
	VkAccelerationStructureKHR tlas;
};

struct RenderContext
{
	std::vector<RenderInstance> instances;
	SceneRenderData scene;
	CameraRenderData camera;
	LightRenderData light;
	RaytracingData raytracing;
	float renderScale;
};
