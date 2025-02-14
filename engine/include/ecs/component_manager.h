#pragma once

#include <unordered_map>
#include <ecs/component.h>
#include <ecs/entity.h>
#include <ecs/ecs_constants.h>
#include <bitset>
#include <span>

typedef u8 ComponentType;

class ComponentManager
{
public:
	ComponentManager();
    ComponentType RegisterComponent(std::string& name); // Assign a ComponentType to component
	ComponentType GetComponentType(std::string& name);
	std::span<Component>& GetComponentsOfType(ComponentType type);
    Component& GetComponent(EntityHandle entity, ComponentType type);
    ComponentHandle AddComponent(EntityHandle entity, ComponentType type);
    void RemoveComponent(ComponentHandle handle);

  private:
	std::unordered_map<std::string, ComponentType> mComponentTypes;
	std::unordered_map<ComponentType, std::array<Component, MAX_COMPONENT_INSTANCES>*> mComponentArrays;
	ComponentType mNextComponentType;
};

