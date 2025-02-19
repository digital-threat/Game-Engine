#pragma once

#include <ecs/entity.h>
#include <ecs/ecs_constants.h>
#include <array>
#include <unordered_map>
#include <cassert>

struct ComponentHandle
{
	u32 handle;
};

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void EntityDestroyed(Entity entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray
{
public:
	ComponentHandle AddComponent(Entity entity, T& component)
	{
		assert(!mEntityToIndexMap.contains(entity) && "Cannot add component: entity already has a component of this type");
		assert(mSize < MAX_ENTITIES && "Cannot add component: max components reached");
		// TODO(Sergei): Assert entity handle is valid

		ComponentHandle handle = { mSize };

		mEntityToIndexMap[entity] = handle;
		mIndexToEntityMap[handle] = entity;
		mComponentArray[handle] = component;

		mSize++;

		return handle;
	}

	T& GetComponent(Entity entity)
	{
		assert(mEntityToIndexMap.contains(entity) && "Cannot get component: invalid entity or entity doesn't have a component of this type");
		// TODO(Sergei): Assert entity handle is valid

		return mComponentArray[mEntityToIndexMap[entity]];
	}

	void RemoveComponent(Entity entity)
	{
		assert(mEntityToIndexMap.contains(entity) && "Cannot remove component: invalid entity or entity doesn't have a component of this type");
		// TODO(Sergei): Assert entity handle is valid

		ComponentHandle removedElement = mEntityToIndexMap[entity];
		ComponentHandle lastElement = { mSize - 1 };
		mComponentArray[removedElement] = std::move(mComponentArray[lastElement]);

		Entity lastElementEntity = mIndexToEntityMap[lastElement];
		mEntityToIndexMap[lastElementEntity] = removedElement;
		mIndexToEntityMap[removedElement] = lastElementEntity;

		mEntityToIndexMap.erase(entity);
		mIndexToEntityMap.erase(lastElement);

		mSize++;
	}

	void EntityDestroyed(Entity entity) override
	{
		if (mEntityToIndexMap.contains(entity))
		{
			RemoveComponent(entity);
		}
	}

private:
	std::array<T, MAX_ENTITIES> mComponentArray;
	std::unordered_map<Entity, ComponentHandle> mEntityToIndexMap;
	std::unordered_map<ComponentHandle, Entity> mIndexToEntityMap;
    u32 mSize;
};
