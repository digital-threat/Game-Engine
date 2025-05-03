#pragma once

#include <glm/glm.hpp>
#include <ecs/coordinator.h>
#include <mesh_structs.h>

struct Scene
{
	std::vector<MeshHandle> meshes;

	glm::vec3 mainLightColor;
	glm::vec3 mainLightPosition;
	float mainLightIntensity;

	glm::vec3 skyColor;

	Coordinator coordinator;
};
