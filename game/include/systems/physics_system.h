#pragma once

class ComponentManager;
class EntityManager;

class PhysicsSystem
{
public:
	void Update(EntityManager& entityManager, ComponentManager& componentManager, float deltaTime);
};