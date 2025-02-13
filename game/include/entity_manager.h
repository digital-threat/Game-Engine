#pragma once

#include "entity.h"

class EntityManager
{
public:
	Entity & CreateEntity();
	void DeleteEntity(Entity *pEntity);
	std::vector<Entity *> All();
	size_t Count();
	Entity * GetById(i32 pId);

private:
	i32 nextId = 1;
	std::unordered_map<i32, Entity *> mEntities;
};
