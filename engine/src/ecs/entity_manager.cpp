#include <ecs/entity_manager.h>
#include <ecs/constants.h>
#include <ecs/typedefs.h>
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
	assert(mEntityCount < MAX_ENTITIES);

	Entity entity = mAvailableEntities.front();
	mAvailableEntities.pop();
	mEntityCount++;

	return entity;
}

void EntityManager::DestroyEntity(Entity entity)
{
	assert(entity < MAX_ENTITIES);

	mArchetypes[entity].reset();
	mAvailableEntities.push(entity);
	mEntityCount--;
}

void EntityManager::SetArchetype(Entity entity, Archetype archetype)
{
	assert(entity < MAX_ENTITIES);

	mArchetypes[entity] = archetype;
}

Archetype EntityManager::GetArchetype(Entity entity)
{
	assert(entity < MAX_ENTITIES);

	return mArchetypes[entity];
}

void EntityManager::Each(Archetype archetype, std::function<void(Entity entity)> f)
{
	for (int i = 0; i < mEntityCount; i++)
	{
		if ((mArchetypes[i] & archetype) == archetype)
		{
			f(Entity(i));
		}
	}
}
