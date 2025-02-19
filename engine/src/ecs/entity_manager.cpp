#include <ecs/entity_manager.h>
#include <ecs/ecs_constants.h>
#include <ecs/entity.h>
#include <cassert>

EntityManager::EntityManager()
{
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		mAvailableEntities.emplace(i);
	}
}

Entity EntityManager::CreateEntity()
{
	assert(mEntityCount < MAX_ENTITIES, "Cannot create entity: max entities reached");

	Entity entity = mAvailableEntities.front();
	mAvailableEntities.pop();
	mEntityCount++;

	return entity;
}

void EntityManager::DestroyEntity(Entity entity)
{
	assert(entity.handle < MAX_ENTITIES, "Cannot destroy entity: invalid entity");

	mArchetypes[entity.handle].reset();
	mAvailableEntities.push(entity);
	mEntityCount--;
}

void EntityManager::SetArchetype(Entity entity, Archetype archetype)
{
	assert(entity.handle < MAX_ENTITIES, "Cannot destroy entity: invalid entity");

	mArchetypes[entity.handle] = archetype;
}

Archetype EntityManager::GetArchetype(Entity entity)
{
	assert(entity.handle < MAX_ENTITIES, "Cannot destroy entity: invalid entity");

	return mArchetypes[entity.handle];
}
