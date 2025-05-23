#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

// TODO(Sergei): Deinterleave
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;

	bool operator==(const Vertex& other) const
	{
		return position == other.position &&
			   normal == other.normal &&
			   uv == other.uv;
	}
};
