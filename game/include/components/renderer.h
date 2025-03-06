#pragma once

#include <handles.h>

struct Mesh;

struct Renderer
{
	Mesh* mesh;
	MaterialHandle material;
};