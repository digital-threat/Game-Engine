#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation;
	float scale;
};
