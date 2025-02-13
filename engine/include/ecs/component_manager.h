#pragma once

#include <unordered_map>
#include <ecs/component.h>
#include <ecs/ecs_constants.h>
#include <bitset>

typedef std::bitset<MAX_COMPONENTS> ComponentType;

class ComponentManager
{
public:
	ComponentManager();
    void RegisterComponent(); // Assign a ComponentType to component

private:
	std::unordered_map<ComponentType, Component[MAX_COMPONENTS]> mComponentArrays{};
};

