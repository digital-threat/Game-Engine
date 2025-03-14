#pragma once

class EntityManager;
class ComponentManager;
struct SceneRenderData;

class CameraSystem
{
public:
	static void Update(EntityManager& entityManager, ComponentManager& componentManager, SceneRenderData &context);
	static void OnGUI(EntityManager& entityManager, ComponentManager& componentManager);
};