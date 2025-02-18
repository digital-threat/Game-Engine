#include <ecs/component_array.h>
#include <cassert>

template<typename T>
ComponentHandle ComponentArray<T>::AddComponent(EntityHandle entity, T &component)
{
	assert(!mEntityToIndexMap.contains(entity), "Cannot add component: entity already has a component of this type");
	assert(mSize < MAX_ENTITIES, "Cannot add component: max components reached");
	// TODO(Sergei): Assert entity handle is valid

	ComponentHandle handle = { mSize };

	mEntityToIndexMap[entity] = handle;
	mIndexToEntityMap[handle] = entity;
	mComponentArray[handle] = component;

	mSize++;
}

template<typename T>
T& ComponentArray<T>::GetComponent(EntityHandle entity)
{
	assert(mEntityToIndexMap.contains(entity), "Cannot get component: invalid entity or entity doesn't have a component of this type");
	// TODO(Sergei): Assert entity handle is valid

	return mComponentArray[mEntityToIndexMap[entity]];
}

template<typename T>
void ComponentArray<T>::RemoveComponent(EntityHandle entity)
{
	assert(mEntityToIndexMap.contains(entity), "Cannot remove component: invalid entity or entity doesn't have a component of this type");
	// TODO(Sergei): Assert entity handle is valid

	ComponentHandle removedElement = mEntityToIndexMap[entity];
	ComponentHandle lastElement = { mSize - 1 };
	mComponentArray[removedElement] = std::move(mComponentArray[lastElement]);

	EntityHandle lastElementEntity = mIndexToEntityMap[lastElement];
	mEntityToIndexMap[lastElementEntity] = removedElement;
	mIndexToEntityMap[removedElement] = lastElementEntity;

	mEntityToIndexMap.erase(entity);
	mIndexToEntityMap.erase(lastElement);

	mSize++;
}

