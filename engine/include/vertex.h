#pragma once

#include <glm/vec3.hpp>

struct Vertex
{
	glm::vec3 position;
	float u;
	glm::vec3 normal;
	float v;

	bool operator==(const Vertex& other) const
	{
		return position == other.position &&
			   u == other.u &&
			   v == other.v &&
			   normal == other.normal;
	}
};
