#pragma once

#include <glm/vec3.hpp>
#include "renderer_vk_types.h"
#include <string>

struct Entity
{
	i32 id;
	std::string name = "Default Name";
	glm::vec3 position;
	glm::vec3 rotation;
	float scale = 1;
	MeshAsset* mesh = nullptr;
	VulkanImage* texture = nullptr;

	Entity() = delete;

	Entity(i32 id)
	{
		this->id = id;
	}
};
