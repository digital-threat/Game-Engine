#pragma once

#include <vulkan/vulkan_core.h>
#include <types.h>

VkCommandPoolCreateInfo CommandPoolCreateInfo(u32 pQueueFamilyIndex, VkCommandPoolCreateFlags pFlags = 0);
VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pPool, u32 pCount = 1);

VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags pFlags = 0);
VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer pCmd);

VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags pFlags = 0);

VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags pFlags = 0);
VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 pStageMask, VkSemaphore pSemaphore);

VkImageCreateInfo ImageCreateInfo(VkFormat pFormat, VkImageUsageFlags pUsageFlags, VkExtent3D pExtent);
VkImageViewCreateInfo ImageViewCreateInfo(VkImage pImage, VkFormat pFormat, VkImageAspectFlags pAspectFlags);

VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags pAspectMask);

VkSubmitInfo2 SubmitInfo(const VkCommandBufferSubmitInfo* pCmd, const VkSemaphoreSubmitInfo* pSignalSemaphoreInfo, const VkSemaphoreSubmitInfo* pWaitSemaphoreInfo);

VkRenderingAttachmentInfo ColorAttachmentInfo(VkImageView pView, const VkClearValue* pClear, VkImageLayout pLayout);
VkRenderingAttachmentInfo DepthAttachmentInfo(VkImageView pView, VkImageLayout pLayout);
VkRenderingInfo RenderingInfo(VkExtent2D pExtent, VkRenderingAttachmentInfo* pColorAttachment, VkRenderingAttachmentInfo* pDepthAttachment);

VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();
VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits pStage, VkShaderModule pShaderModule, const char * pEntry);


