#pragma once

class EntityManager;
class ComponentManager;

class TransformGUISystem
{
public:
	static void Update(EntityManager& entityManager, ComponentManager& componentManager);
};