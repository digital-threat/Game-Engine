#include "renderer_vk_images.h"

#include <renderer_vk_buffers.h>
#include <renderer_vk_structures.h>
#include <renderer_vk_types.h>
#include <renderer_vk_utility.h>
#include <stdexcept>


namespace Renderer
{
	void TransitionImageLayout(VkCommandBuffer pCmd, VkImage pImage, VkImageLayout pOldLayout, VkImageLayout pNewLayout)
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;
		barrier.oldLayout = pOldLayout;
		barrier.newLayout = pNewLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = pImage;

		VkImageAspectFlags aspectMask = (pNewLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange = ImageSubresourceRange(aspectMask);

		if (pOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && pNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		}
		else if (pOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && pNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		}
		else if (pOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && pNewLayout == VK_IMAGE_LAYOUT_GENERAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		}
		else if (pOldLayout == VK_IMAGE_LAYOUT_GENERAL && pNewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
			barrier.dstAccessMask = 0;
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
		}
		else
		{
			barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		}

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(pCmd, &dependencyInfo);
	}

	void CopyImage(VkCommandBuffer pCmd, VkImage pSource, VkImage pDestination, VkExtent2D pSrcSize, VkExtent2D pDstSize)
	{
		VkImageBlit2 blitRegion{};
		blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
		blitRegion.pNext = nullptr;
		blitRegion.srcOffsets[1].x = pSrcSize.width;
		blitRegion.srcOffsets[1].y = pSrcSize.height;
		blitRegion.srcOffsets[1].z = 1;
		blitRegion.dstOffsets[1].x = pDstSize.width;
		blitRegion.dstOffsets[1].y = pDstSize.height;
		blitRegion.dstOffsets[1].z = 1;
		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;
		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		VkBlitImageInfo2 blitInfo{};
		blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
		blitInfo.pNext = nullptr;
		blitInfo.dstImage = pDestination;
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.srcImage = pSource;
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		vkCmdBlitImage2(pCmd, &blitInfo);
	}

	void CreateImage(VmaAllocator pAllocator, VkImageCreateInfo pInfo, VmaMemoryUsage pUsage, VkMemoryPropertyFlags pFlags, VulkanImage &outImage)
	{
		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = pUsage;
		allocationInfo.requiredFlags = pFlags;

		if (vmaCreateImage(pAllocator, &pInfo, &allocationInfo, &outImage.image, &outImage.allocation, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image.");
		}
	}

	VulkanImage CreateImage(VkDevice pDevice, VmaAllocator pAllocator, VkExtent3D pSize, VkFormat pFormat, VkImageUsageFlags pUsage, bool pMipmapped)
	{
		VulkanImage image;
		image.format = pFormat;
		image.extent = pSize;

		VkImageCreateInfo imageInfo = ImageCreateInfo(pFormat, pUsage, pSize);
		if (pMipmapped)
		{
			imageInfo.mipLevels = static_cast<u32>(std::floor(std::log2(std::max(pSize.width, pSize.height)))) + 1;
		}

		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		vmaCreateImage(pAllocator, &imageInfo, &allocationInfo, &image.image, &image.allocation, nullptr);

		VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
		if (pFormat == VK_FORMAT_D32_SFLOAT || pFormat == VK_FORMAT_D24_UNORM_S8_UINT || pFormat == VK_FORMAT_D16_UNORM_S8_UINT)
		{
			aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		VkImageViewCreateInfo viewInfo = ImageViewCreateInfo(image.image, pFormat, aspectFlag);
		viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;

		vkCreateImageView(pDevice, &viewInfo, nullptr, &image.imageView);

		return image;
	}

	VulkanImage CreateImage(const VkDevice& pDevice, const VkQueue& pQueue, const ImmediateData& pImmData, const VmaAllocator& pAllocator, void *pData, VkExtent3D pSize, VkFormat pFormat, VkImageUsageFlags pUsage, bool pMipmapped)
	{
		size_t dataSize = pSize.depth * pSize.width * pSize.height * 4;
		VulkanBuffer stagingBuffer = CreateBuffer(pAllocator, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		memcpy(stagingBuffer.info.pMappedData, pData, dataSize);

		VulkanImage image = CreateImage(pDevice, pAllocator, pSize, pFormat, pUsage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, pMipmapped);

		auto func = [&](VkCommandBuffer cmd)
		{
			TransitionImageLayout(cmd, image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkBufferImageCopy copyRegion{};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = pSize;

			vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			TransitionImageLayout(cmd, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		};

		ImmediateSubmit(pDevice, pQueue, pImmData, func);

		DestroyBuffer(pAllocator, stagingBuffer);

		return image;
	}

	void DestroyImage(const VkDevice& pDevice, const VmaAllocator& pAllocator, const VulkanImage& pImage)
	{
		vkDestroyImageView(pDevice, pImage.imageView, nullptr);
		vmaDestroyImage(pAllocator, pImage.image, pImage.allocation);
	}

	VkFormat FindSupportedFormat(const VkPhysicalDevice& pPhysicalDevice, const std::span<VkFormat>& pCandidates, VkImageTiling pTiling, VkFormatFeatureFlags pFeatures)
	{
		for (VkFormat format : pCandidates)
		{
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(pPhysicalDevice, format, &properties);

			if (pTiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & pFeatures) == pFeatures)
			{
				return format;
			}
			if (pTiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & pFeatures) == pFeatures)
			{
				return format;
			}
		}

		throw std::runtime_error("Failed to find supported format.");
	}
}
