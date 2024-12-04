#pragma once

#include "entity.h"

class EntityManager
{
public:
	Entity& CreateEntity();
	void DeleteEntity(Entity*  pEntity);
	std::vector<Entity*> All();
	size_t EntityCount();

private:
	int nextId = 1;
	std::unordered_map<int, Entity*> mEntities;
};
