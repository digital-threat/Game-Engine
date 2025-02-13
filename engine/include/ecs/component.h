#pragma once

#include <types.h>

struct ComponentHandle
{
	u16 handle;
};

struct Component
{
  	std::bitset<MAX_COMPONENTS> type;
	ComponentHandle handle;
};



