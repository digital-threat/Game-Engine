#pragma once

#include <renderer_vk_types.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace Renderer
{
	void TransitionImageLayout(VkCommandBuffer pCmd, VkImage pImage, VkImageLayout pOldLayout, VkImageLayout pNewLayout);
	void CopyImage(VkCommandBuffer pCmd, VkImage pSource, VkImage pDestination, VkExtent2D pSrcSize, VkExtent2D pDstSize);
	void CreateImage(VmaAllocator pAllocator, VkImageCreateInfo pInfo, VmaMemoryUsage pUsage, VkMemoryPropertyFlags pFlags, VulkanImage &outImage);
}
