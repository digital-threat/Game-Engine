#pragma once

#include <ecs/entity.h>
#include <ecs/ecs_constants.h>
#include <array>
#include <unordered_map>

struct ComponentHandle
{
	u32 handle;
};

template<typename T>
class ComponentArray
{
public:
	ComponentHandle AddComponent(EntityHandle entity, T& component);
	T& GetComponent(EntityHandle entity);
	void RemoveComponent(EntityHandle entity);

private:
	std::array<T, MAX_ENTITIES> mComponentArray;
	std::unordered_map<EntityHandle, ComponentHandle> mEntityToIndexMap;
	std::unordered_map<ComponentHandle, EntityHandle> mIndexToEntityMap;
    u32 mSize;
};
