#pragma once

#include <ecs/typedefs.h>
#include <ecs/constants.h>

#include <unordered_map>
#include <array>
#include <cassert>

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
	Component AddComponent(Entity entity, T& component)
	{
		assert(!mEntityToIndexMap.contains(entity));
		assert(mSize < MAX_ENTITIES);
		// TODO(Sergei): Assert entity handle is valid

		Component handle = { mSize };

		mEntityToIndexMap[entity] = handle;
		mIndexToEntityMap[handle] = entity;
		mComponentArray[handle] = component;

		mSize++;

		return handle;
	}

	T& GetComponent(Entity entity)
	{
		assert(mEntityToIndexMap.contains(entity));
		// TODO(Sergei): Assert entity handle is valid

		return mComponentArray[mEntityToIndexMap[entity]];
	}

	void RemoveComponent(Entity entity)
	{
		assert(mEntityToIndexMap.contains(entity));
		// TODO(Sergei): Assert entity handle is valid

		Component removedElement = mEntityToIndexMap[entity];
		Component lastElement = { mSize - 1 };
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

public:
	std::array<T, MAX_ENTITIES> mComponentArray;
	u32 mSize;

private:
	std::unordered_map<Entity, Component> mEntityToIndexMap;
	std::unordered_map<Component, Entity> mIndexToEntityMap;
};
