#pragma once

#include <ecs/typedefs.h>
#include <ecs/entity_manager.h>
#include <ecs/component_manager.h>

class Coordinator
{
public:
	Entity CreateEntity()
	{
		return mEntityManager.CreateEntity();
	}

	void DestroyEntity(Entity entity)
	{
		mEntityManager.DestroyEntity(entity);
		mComponentManager.EntityDestroyed(entity);
	}

	void Each(Archetype archetype, std::function<void(Entity entity)> f)
	{
		mEntityManager.Each(archetype, f);
	}

	template<typename T>
	void RegisterComponent()
	{
		mComponentManager.RegisterComponent<T>();
	}

	template<typename T>
	void AddComponent(Entity entity, T component)
	{
		mComponentManager.AddComponent<T>(entity, component);

		auto archetype = mEntityManager.GetArchetype(entity);
		archetype.set(mComponentManager.GetComponentType<T>(), true);
		mEntityManager.SetArchetype(entity, archetype);
	}

	template<typename T>
	void RemoveComponent(Entity entity)
	{
		mComponentManager.RemoveComponent<T>(entity);

		auto archetype = mEntityManager.GetArchetype(entity);
		archetype.set(mComponentManager.GetComponentType<T>(), false);
		mEntityManager.SetArchetype(entity, archetype);
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return mComponentManager.GetComponent<T>(entity);
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		return mComponentManager.GetComponentType<T>();
	}

public:
	ComponentManager mComponentManager;
	EntityManager mEntityManager;
};

