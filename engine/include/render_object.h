#pragma once

#include <mesh_structs.h>
#include <glm/mat4x4.hpp>

struct RenderObject
{
	glm::mat4 transform;
	MeshHandle meshHandle;
};