#pragma once

#include <types.h>

class EntityManager;
class ComponentManager;
struct SceneRenderData;

class CameraSystem
{
public:
	static void Update(EntityManager& entityManager, ComponentManager& componentManager, SceneRenderData &context, f32 deltaTime);
	static void OnGUI(EntityManager& entityManager, ComponentManager& componentManager);
};