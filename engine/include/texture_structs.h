#pragma once

#include <vk_structs.h>
#include <vulkan/vulkan_core.h>

struct Texture
{
	VulkanImage image;
	VkSampler sampler;
};
