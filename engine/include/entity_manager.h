#pragma once

#include "entity.h"

class EntityManager
{
private:
	int nextId = 1;
	
public:
	Entity* CreateEntity();
	void DeleteEntity(Entity*  pEntity);
	std::vector<Entity*> All();

	std::unordered_map<int, Entity* > entities;
};
