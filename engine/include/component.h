#pragma once

#include <renderer_vk_types.h>

class Component
{
public:
	virtual void Update() = 0;
	virtual void Render(RenderContext& context, ModelRenderData& renderData) = 0;
	virtual void OnGUI() = 0;
};
