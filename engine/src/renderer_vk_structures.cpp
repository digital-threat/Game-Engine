#include "renderer_vk_structures.h"


namespace Renderer
{
	VkCommandPoolCreateInfo CommandPoolCreateInfo(const u32 pQueueFamilyIndex, const VkCommandPoolCreateFlags pFlags)
	{
		VkCommandPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.queueFamilyIndex = pQueueFamilyIndex;
		info.flags = pFlags;
		return info;
	}

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(const VkCommandPool pPool, const u32 pCount)
	{
		VkCommandBufferAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = nullptr;
		info.commandPool = pPool;
		info.commandBufferCount = pCount;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		return info;
	}

	VkCommandBufferBeginInfo CommandBufferBeginInfo(const VkCommandBufferUsageFlags pFlags)
	{
		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.pInheritanceInfo = nullptr;
		info.flags = pFlags;
		return info;
	}

	VkCommandBufferSubmitInfo CommandBufferSubmitInfo(const VkCommandBuffer pCmd)
	{
		VkCommandBufferSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		info.pNext = nullptr;
		info.commandBuffer = pCmd;
		info.deviceMask = 0;
		return info;
	}

	VkFenceCreateInfo FenceCreateInfo(const VkFenceCreateFlags pFlags)
	{
		VkFenceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = pFlags;
		return info;
	}

	VkSemaphoreCreateInfo SemaphoreCreateInfo(const VkSemaphoreCreateFlags pFlags)
	{
		VkSemaphoreCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = pFlags;
		return info;
	}

	VkSemaphoreSubmitInfo SemaphoreSubmitInfo(const VkPipelineStageFlags2 pStageMask, const VkSemaphore pSemaphore)
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

	// NOTE(Sergei): Linear tiling is for textures that need to be read on the CPU.
	VkImageCreateInfo ImageCreateInfo(const VkFormat pFormat, const VkImageUsageFlags pUsageFlags, const VkExtent3D pExtent)
	{
		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = pFormat;
		info.extent = pExtent;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = pUsageFlags;
		return info;
	}

	VkImageSubresourceRange ImageSubresourceRange(const VkImageAspectFlags pAspectMask)
	{
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = pAspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		return subresourceRange;
	}

	VkImageViewCreateInfo ImageViewCreateInfo(const VkImage pImage, const VkFormat pFormat, const VkImageAspectFlags pAspectFlags)
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

	VkSubmitInfo2 SubmitInfo(const VkCommandBufferSubmitInfo *pCmd, const VkSemaphoreSubmitInfo *pSignalSemaphoreInfo, const VkSemaphoreSubmitInfo *pWaitSemaphoreInfo)
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

	VkRenderingAttachmentInfo RenderingAttachmentInfo(const VkImageView pView, const VkClearValue *pClear, const VkImageLayout pLayout)
	{
		VkRenderingAttachmentInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		info.pNext = nullptr;
		info.imageView = pView;
		info.imageLayout = pLayout;
		info.loadOp = pClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		if (pClear)
		{
			info.clearValue = *pClear;
		}
		return info;
	}

	VkRenderingInfo RenderingInfo(VkExtent2D pExtent, VkRenderingAttachmentInfo *pColorAttachment, VkRenderingAttachmentInfo *pDepthAttachment)
	{
		VkRenderingInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.pNext = nullptr;
		info.renderArea = VkRect2D{VkOffset2D {0, 0}, pExtent};
		info.layerCount = 1;
		info.colorAttachmentCount = 1;
		info.pColorAttachments = pColorAttachment;
		info.pDepthAttachment = pDepthAttachment;
		info.pStencilAttachment = nullptr;
		return info;
	}

	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo()
	{
		VkPipelineLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.setLayoutCount = 0;
		info.pSetLayouts = nullptr;
		info.pushConstantRangeCount = 0;
		info.pPushConstantRanges = nullptr;
		return info;
	}

	VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits pStage, VkShaderModule pShaderModule, const char *pEntry)
	{
		VkPipelineShaderStageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.stage = pStage;
		info.module = pShaderModule;
		info.pName = pEntry;
		return info;
	}
}

