#pragma once

#include "renderer_vk_types.h"
#include "component.h"
#include <string>

class Entity
{
public:
	Entity() = delete;
	Entity(i32 id);

	void Update();
	void Render(RenderContext &renderContext);
	void OnGUI();

	void AddComponent(Component* component);

public:
	std::string mName = "Default Name";
	i32 id;

private:
	std::vector<Component*> mComponents;
};
