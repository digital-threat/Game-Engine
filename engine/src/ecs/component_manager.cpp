#pragma once

#include <cassert>
#include <ecs/component_manager.h>


ComponentManager::ComponentManager()
{
	mComponentArrays.reserve(MAX_COMPONENTS);
	mNextComponentType = 0;
}

void ComponentManager::RegisterComponent(std::string& name)
{
	assert(!mComponentTypes.contains(name), "Cannot register a component: component already registered");
	assert(mNextComponentType < MAX_COMPONENTS, "Cannot register a component: MAX_COMPONENTS already reached");

	mComponentTypes[name] = mNextComponentType;

	// NOTE(Sergei): This makes registering a component expensive
	// I could preallocate all arrays but that may consume a lot of memory depending on MAX_COMPONENTS.
	mComponentArrays.emplace(mNextComponentType, std::array<Component, MAX_ENTITIES>());

	mNextComponentType++;
}

ComponentType ComponentManager::GetComponentType(std::string &name)
{
	assert(mComponentTypes.contains(name), "Cannot retrieve component type: component not registered");
	return mComponentTypes[name];
}

std::array<Component, MAX_ENTITIES>& ComponentManager::GetComponentsOfType(std::string& name)
{
	auto type = GetComponentType(name);
	assert(mComponentArrays.contains(type), "Cannot retrieve component array: component not registered");
	return mComponentArrays[type];
}

Component& ComponentManager::GetComponent(EntityHandle entity, std::string& name)
{
	auto type = GetComponentType(name);
	assert(mComponentArrays.contains(type), "Cannot retrieve component array: component not registered");
	// TODO(Sergei): Assert entity handle is valid
	return mComponentArrays[type][entity.handle];
}

void ComponentManager::AddComponent(EntityHandle entity, Component& component)
{
	assert(mComponentArrays.contains(component.type), "Cannot retrieve component array: component not registered");
	// TODO(Sergei): Assert entity handle is valid
	mComponentArrays[component.type][entity.handle] = component;

}

void ComponentManager::RemoveComponent(EntityHandle entity, std::string& name)
{
}
