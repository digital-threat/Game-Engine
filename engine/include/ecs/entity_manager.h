#pragma once

#include <ecs/ecs_constants.h>
#include <ecs/ecs_typedefs.h>
#include <types.h>
#include <bitset>
#include <queue>
#include <array>

struct Entity;

class EntityManager
{
public:
	EntityManager();
	Entity CreateEntity();
	void DestroyEntity(Entity entity);
	void SetArchetype(Entity entity, Archetype archetype);
	Archetype GetArchetype(Entity entity);

private:
	std::queue<Entity> mAvailableEntities;
	std::array<Archetype, MAX_ENTITIES> mArchetypes;
	u32 mEntityCount;
};
