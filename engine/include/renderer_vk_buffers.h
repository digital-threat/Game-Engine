#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "renderer_vk_types.h"

namespace Renderer
{
	VulkanBuffer CreateBuffer(VmaAllocator pAllocator, size_t pAllocSize, VkBufferUsageFlags pUsage, VmaMemoryUsage pMemoryUsage);
	void DestroyBuffer(VmaAllocator pAllocator, const VulkanBuffer& pBuffer);
}
