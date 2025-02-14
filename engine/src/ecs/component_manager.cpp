#pragma once

#include <cassert>
#include <ecs/component_manager.h>


ComponentManager::ComponentManager()
{
	mComponentArrays.reserve(MAX_COMPONENTS);
	mNextComponentType = 0;
}

ComponentType ComponentManager::RegisterComponent(std::string& name)
{
	assert(!mComponentTypes.contains(name), "Cannot register a component: component already registered");
	assert(mNextComponentType < MAX_COMPONENTS, "Cannot register a component: MAX_COMPONENTS already reached");

	mComponentTypes[name] = mNextComponentType;

	// NOTE(Sergei): This makes registering a component expensive
	// I could preallocate all arrays but that may consume a lot of memory depending on MAX_COMPONENTS.
	mComponentArrays[mNextComponentType] = new std::array<Component, MAX_COMPONENT_INSTANCES>();

	mNextComponentType++;
}

ComponentType ComponentManager::GetComponentType(std::string &name)
{
}

std::span<Component> & ComponentManager::GetComponentsOfType(ComponentType type)
{
}

Component & ComponentManager::GetComponent(EntityHandle entity, ComponentType type)
{
}

ComponentHandle ComponentManager::AddComponent(EntityHandle entity, ComponentType type)
{
}

void ComponentManager::RemoveComponent(ComponentHandle handle)
{
}
