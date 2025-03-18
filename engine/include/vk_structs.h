#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

struct VulkanImage
{
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
	VkExtent3D extent;
	VkFormat format;
};

struct VulkanBuffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo info;
};