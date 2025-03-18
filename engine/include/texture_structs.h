#pragma once

#include <vulkan/vulkan_core.h>

struct Texture
{
	VkImageView view;
	VkSampler sampler;
};
