#pragma once

class ComponentManager;
class EntityManager;

class PhysicsSystem
{
public:
	static void Update(EntityManager& entityManager, ComponentManager& componentManager, float deltaTime);
};