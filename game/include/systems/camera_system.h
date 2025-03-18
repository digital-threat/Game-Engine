#pragma once

#include <types.h>

struct CameraRenderData;
class EntityManager;
class ComponentManager;

class CameraSystem
{
public:
	static void Update(EntityManager& entityManager, ComponentManager& componentManager, CameraRenderData& context, f32 deltaTime);
	static void OnGUI(EntityManager& entityManager, ComponentManager& componentManager);
};