#pragma once

#include <glm/vec3.hpp>
#include "renderer_vk_types.h"
#include <string>

struct Entity
{
	int id;
	std::string name = "Default Name";
	glm::vec3 position;
	glm::vec3 rotation;
	float scale = 1;
	Renderer::MeshAsset* mesh = nullptr;

	Entity() = delete;

	Entity(int id)
	{
		this->id = id;
	}
};
