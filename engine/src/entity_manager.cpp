#include "entity_manager.h"

#include <iostream>


Entity* EntityManager::CreateEntity()
{
	auto entity = new Entity(nextId);
	nextId++;
	entities[entity->id] = entity;
	return entity;
}

void EntityManager::DeleteEntity(Entity* pEntity)
{
	entities.erase(pEntity->id);
	delete pEntity;
}

std::vector<Entity *> EntityManager::All()
{
	std::vector<Entity*> allEntities;
	for (auto entry : entities)
	{
		allEntities.push_back(entry.second);
	}
	return allEntities;
}
