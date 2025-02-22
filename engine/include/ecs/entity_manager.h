#pragma once

#include <ecs/constants.h>
#include <ecs/typedefs.h>

#include <types.h>
#include <bitset>
#include <queue>
#include <array>
#include <functional>

struct Entity;

class EntityManager
{
public:
	EntityManager();
	Entity CreateEntity();
	void DestroyEntity(Entity entity);
	void SetArchetype(Entity entity, Archetype archetype);
	Archetype GetArchetype(Entity entity);
	void Each(Archetype archetype, std::function<void(Entity entity)> f);

private:
	std::queue<Entity> mAvailableEntities;
	std::array<Archetype, MAX_ENTITIES> mArchetypes;
	u32 mEntityCount;
};
