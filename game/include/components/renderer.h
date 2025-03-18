#pragma once

#include <material_structs.h>

struct Mesh;

struct Renderer
{
	Mesh* mesh;
	MaterialHandle material;
};