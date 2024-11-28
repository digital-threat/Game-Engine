#include "renderer_vk_structures.h"


VkCommandPoolCreateInfo Renderer::CommandPoolCreateInfo(const u32 pQueueFamilyIndex, const VkCommandPoolCreateFlags pFlags)
{
	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.queueFamilyIndex = pQueueFamilyIndex;
	info.flags = pFlags;
	return info;
}

VkCommandBufferAllocateInfo Renderer::CommandBufferAllocateInfo(const VkCommandPool pPool, const u32 pCount)
{
	VkCommandBufferAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;
	info.commandPool = pPool;
	info.commandBufferCount = pCount;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	return info;
}

VkCommandBufferBeginInfo Renderer::CommandBufferBeginInfo(const VkCommandBufferUsageFlags pFlags)
{
	VkCommandBufferBeginInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;
	info.pInheritanceInfo = nullptr;
	info.flags = pFlags;
	return info;
}

VkCommandBufferSubmitInfo Renderer::CommandBufferSubmitInfo(const VkCommandBuffer pCmd)
{
	VkCommandBufferSubmitInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	info.pNext = nullptr;
	info.commandBuffer = pCmd;
	info.deviceMask = 0;
	return info;
}

VkFenceCreateInfo Renderer::FenceCreateInfo(const VkFenceCreateFlags pFlags)
{
	VkFenceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = pFlags;
	return info;
}

VkSemaphoreCreateInfo Renderer::SemaphoreCreateInfo(const VkSemaphoreCreateFlags pFlags)
{
	VkSemaphoreCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = pFlags;
	return info;
}

VkSemaphoreSubmitInfo Renderer::SemaphoreSubmitInfo(const VkPipelineStageFlags2 pStageMask, const VkSemaphore pSemaphore)
{
	VkSemaphoreSubmitInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	info.pNext = nullptr;
	info.semaphore = pSemaphore;
	info.stageMask = pStageMask;
	info.deviceIndex = 0;
	info.value = 1;
	return info;
}

VkImageSubresourceRange Renderer::ImageSubresourceRange(VkImageAspectFlags pAspectMask)
{
	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = pAspectMask;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	return subresourceRange;
}

VkImageViewCreateInfo Renderer::ImageViewCreateInfo(VkImage pImage, VkFormat pFormat, VkImageAspectFlags pAspectFlags)
{
	VkImageViewCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = nullptr;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.image = pImage;
	info.format = pFormat;
	info.subresourceRange.aspectMask = pAspectFlags;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;
	return info;
}

VkSubmitInfo2 Renderer::SubmitInfo(VkCommandBufferSubmitInfo *pCmd, VkSemaphoreSubmitInfo *pSignalSemaphoreInfo, VkSemaphoreSubmitInfo *pWaitSemaphoreInfo)
{
	VkSubmitInfo2 info{};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	info.pNext = nullptr;
	info.waitSemaphoreInfoCount = pWaitSemaphoreInfo == nullptr ? 0 : 1;
	info.pWaitSemaphoreInfos = pWaitSemaphoreInfo;
	info.signalSemaphoreInfoCount = pSignalSemaphoreInfo == nullptr ? 0 : 1;
	info.pSignalSemaphoreInfos = pSignalSemaphoreInfo;
	info.commandBufferInfoCount = 1;
	info.pCommandBufferInfos = pCmd;
	return info;
}

