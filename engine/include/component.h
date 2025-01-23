#pragma once

#include <renderer_vk_types.h>

class Entity;

enum class ComponentType
{
	TRANSFORM,
	MESH,
	LIGHT
};

class Component
{
public:
	Entity& parent;
	ComponentType mComponentType;

public:
	Component() = delete;
	Component(Entity& parent);

public:
	virtual void Update() = 0;
	virtual void Render(RenderContext& context, ModelRenderData& renderData) = 0;
	virtual void OnGUI() = 0;
};
