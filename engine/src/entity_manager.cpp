#include "entity_manager.h"

#include <iostream>


Entity& EntityManager::CreateEntity()
{
	auto entity = new Entity(nextId);
	nextId++;
	mEntities[entity->id] = entity;
	return *entity;
}

void EntityManager::DeleteEntity(Entity *pEntity)
{
	mEntities.erase(pEntity->id);
	delete pEntity;
}

std::vector<Entity*> EntityManager::All()
{
	std::vector<Entity*> allEntities;
	for (auto entry : mEntities)
	{
		allEntities.push_back(entry.second);
	}
	return allEntities;
}

size_t EntityManager::EntityCount()
{
	return mEntities.size();
}
