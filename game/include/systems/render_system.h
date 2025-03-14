#pragma once

class EntityManager;
class ComponentManager;
struct RenderContext;

class RenderSystem
{
public:
	static void Update(EntityManager& entityManager, ComponentManager& componentManager, RenderContext &context);
};
