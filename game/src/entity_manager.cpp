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

size_t EntityManager::Count()
{
	return mEntities.size();
}

Entity * EntityManager::GetById(i32 pId)
{
	auto it = mEntities.find(pId);
	if (it != mEntities.end())
	{
		return it->second;
	}

	return nullptr;
}
