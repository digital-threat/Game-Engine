#pragma once

#include <unordered_map>
#include <ecs/component.h>
#include <ecs/entity.h>
#include <ecs/ecs_constants.h>
#include <bitset>
#include <span>

class ComponentManager
{
public:
	ComponentManager();
    void RegisterComponent(std::string& name);
	ComponentType GetComponentType(std::string& name);
	std::array<Component, MAX_COMPONENT_INSTANCES>& GetComponentsOfType(std::string& name);
    Component& GetComponent(EntityHandle entity, std::string& name);
    void AddComponent(EntityHandle entity, Component& component);
    void RemoveComponent(EntityHandle entity, std::string& name);

  private:
	std::unordered_map<std::string, ComponentType> mComponentTypes;
	std::unordered_map<ComponentType, std::array<Component, MAX_COMPONENT_INSTANCES>> mComponentArrays;
	ComponentType mNextComponentType;
};

