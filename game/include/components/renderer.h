#pragma once

#include <material_structs.h>

struct GpuMesh;

struct Renderer
{
	GpuMesh* mesh;
	MaterialHandle material;
};