#pragma once

#include <ecs/ecs_constants.h>
#include <types.h>
#include <bitset>
#include <queue>
#include <array>

struct EntityHandle;

typedef std::bitset<MAX_COMPONENTS> Archetype;

class EntityManager
{
public:
	EntityManager();
	EntityHandle CreateEntity();
	void DestroyEntity(EntityHandle handle);
	void SetArchetype(EntityHandle handle, Archetype archetype);
	Archetype GetArchetype(EntityHandle handle);

private:
	std::queue<EntityHandle> mAvailableHandles;
	std::array<Archetype, MAX_ENTITIES> mArchetypes;
	u32 mEntityCount;
};
