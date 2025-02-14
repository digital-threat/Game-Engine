#pragma once

#include <types.h>
#include <bitset>
#include <ecs/ecs_constants.h>

struct ComponentHandle
{
	u16 handle;
};

struct Component
{
  	std::bitset<MAX_COMPONENTS> type;
	ComponentHandle handle;
};



