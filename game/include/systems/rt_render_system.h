#pragma once


struct Scene;
class EntityManager;
class ComponentManager;

class RtRenderSystem
{
public:
	static void Update(Engine& engine, EntityManager& entityManager, ComponentManager& componentManager, Scene& scene);
};
