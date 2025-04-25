#pragma once

#include <acceleration_structures.h>
#include <ecs/coordinator.h>

struct Scene
{
	Scene(Engine& engine);

	TLAS tlas;
	std::unordered_map<MeshHandle, BLAS> blasMap;
	RaytracingBuilder rtBuilder;
	std::vector<MeshHandle> meshes;

	glm::vec3 mainLightColor;
	glm::vec3 mainLightPosition;
	float mainLightIntensity;

	Coordinator coordinator;

	VkDeviceAddress GetBlasDeviceAddress(u32 handle);

	void CreateBlas();

	void CreateTlas();
};
