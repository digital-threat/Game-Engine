#pragma once

#include <ecs/entity.h>
#include <ecs/constants.h>
#include <ecs/component_array.h>

#include <unordered_map>
#include <typeinfo>

class ComponentManager
{
public:
	ComponentManager()
	{
		mComponentArrays.reserve(MAX_COMPONENTS);
		mNextComponentType = 0;
	}

	template<typename T>
	void RegisterComponent()
	{
		std::string name = typeid(T).name();

		assert(!mComponentTypes.contains(name) && "Cannot register a component: component already registered");
		assert(mNextComponentType < MAX_COMPONENTS && "Cannot register a component: MAX_COMPONENTS already reached");

		mComponentTypes[name] = mNextComponentType;

		// NOTE(Sergei): Alternatively could preallocate all arrays instead.
		mComponentArrays.emplace(mNextComponentType, ComponentArray<T>());

		mNextComponentType++;
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		std::string name = typeid(T).name();
		assert(mComponentTypes.contains(name) && "Cannot retrieve component type: component not registered");
		return mComponentTypes[name];
	}

	template<typename T>
	ComponentArray<T>& GetComponentsOfType()
	{
		auto type = GetComponentType<T>();
		assert(mComponentArrays.contains(type) && "Cannot retrieve component array: component not registered");
		return mComponentArrays[type];
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return GetComponentsOfType<T>().GetComponent(entity);
	}

	template<typename T>
	void AddComponent(Entity entity, T& component)
	{
		GetComponentsOfType<T>().AddComponent(entity, component);
	}

	template<typename T>
    void RemoveComponent(Entity entity)
	{
		GetComponentsOfType<T>().RemoveComponent(entity);
	}

	void EntityDestroyed(Entity entity)
	{
		for (auto const& pair : mComponentArrays)
		{
			auto const& component = pair.second;

			component->EntityDestroyed(entity);
		}
	}

  private:
	std::unordered_map<std::string, ComponentType> mComponentTypes;
	std::unordered_map<ComponentType, IComponentArray*> mComponentArrays;
	ComponentType mNextComponentType;
};

