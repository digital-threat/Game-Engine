#pragma once

#include <glm/vec3.hpp>
#include "renderer_vk_types.h"
#include <string>

struct Entity
{
	std::string name;
	glm::vec3 position;
	glm::vec3 rotation;
	float scale;
	MeshAsset* mesh;
};
