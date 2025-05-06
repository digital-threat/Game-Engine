#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Camera
{
	glm::vec3 position;
	glm::vec2 prevMousePos;
	float yaw;
	float pitch;
	float speed;
	float sensitivity;
	float fov;
};
