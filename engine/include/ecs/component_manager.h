#pragma once

#include <ecs/constants.h>
#include <ecs/component_array.h>
#include <ecs/typedefs.h>

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

		assert(!mComponentTypes.contains(name));
		assert(mNextComponentType < MAX_COMPONENTS);

		mComponentTypes[name] = mNextComponentType;

		// NOTE(Sergei): Alternatively could preallocate all arrays instead.
		mComponentArrays[mNextComponentType] = new ComponentArray<T>();

		mNextComponentType++;
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		std::string name = typeid(T).name();
		assert(mComponentTypes.contains(name));
		return mComponentTypes[name];
	}

	template<typename T>
	ComponentArray<T>& GetComponentsOfType()
	{
		auto type = GetComponentType<T>();
		assert(mComponentArrays.contains(type));
		return *dynamic_cast<ComponentArray<T>*>(mComponentArrays[type]);
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

