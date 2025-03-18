#pragma once

#include <types.h>
#include <glm/vec4.hpp>

#define MAX_LIGHTS 8

enum class LightType : u8
{
	DIRECTIONAL,
	POINT,
	SPOT,
};

struct LightData
{
	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 spotDirection;
	glm::vec4 attenuation;
};
