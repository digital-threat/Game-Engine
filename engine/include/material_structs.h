#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <texture_structs.h>
#include <types.h>

struct MaterialHandle
{
	u16 index = -1;
};

struct Material
{
	std::string name;
	MaterialHandle handle;
	VkBuffer dataBuffer;
	float shininess;
	Texture textures[2];
	VkDescriptorSet materialSet;
};
