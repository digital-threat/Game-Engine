#pragma once

#include <vulkan/vulkan_core.h>
#include "types.h"

namespace Renderer
{
	VkCommandPoolCreateInfo CommandPoolCreateInfo(u32 pQueueFamilyIndex, VkCommandPoolCreateFlags pFlags = 0);
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pPool, u32 pCount = 1);

	VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags pFlags = 0);
	VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer pCmd);

	VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags pFlags = 0);

	VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags pFlags = 0);
	VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 pStageMask, VkSemaphore pSemaphore);

	VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags pAspectMask);

	VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* pCmd, VkSemaphoreSubmitInfo* pSignalSemaphoreInfo, VkSemaphoreSubmitInfo* pWaitSemaphoreInfo);
}
