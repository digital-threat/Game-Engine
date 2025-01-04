#pragma once

#include "renderer_vk_types.h"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include <span>

class Engine;

void CreateImage(VmaAllocator pAllocator, VkImageCreateInfo pInfo, VmaMemoryUsage pUsage, VkMemoryPropertyFlags pFlags, VulkanImage &outImage);
VulkanImage CreateImage(VkExtent3D pSize, VkFormat pFormat, VkImageUsageFlags pUsage, bool pMipmapped = false);
VulkanImage CreateImage(const Engine& pEngine, void* pData, VkExtent3D pSize, VkFormat pFormat, VkImageUsageFlags pUsage, bool pMipmapped = false);
void DestroyImage(const VulkanImage& pImage);

void TransitionImageLayout(VkCommandBuffer pCmd, VkImage pImage, VkImageLayout pOldLayout, VkImageLayout pNewLayout);
void CopyImage(VkCommandBuffer pCmd, VkImage pSource, VkImage pDestination, VkExtent2D pSrcSize, VkExtent2D pDstSize);
VkFormat FindSupportedFormat(const VkPhysicalDevice& pPhysicalDevice, const std::span<VkFormat> &pCandidates, VkImageTiling pTiling, VkFormatFeatureFlags pFeatures);

inline bool HasStencilComponent(VkFormat pFormat)
{
	return pFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || pFormat == VK_FORMAT_D24_UNORM_S8_UINT;
}
