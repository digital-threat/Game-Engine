#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform
{
	glm::vec3 mPosition;
	glm::quaternion mRotation;
	float mScale;
};
