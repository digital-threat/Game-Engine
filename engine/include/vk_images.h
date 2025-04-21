#pragma once

#include <span>
#include <vk_mem_alloc.h>
#include <vk_structs.h>
#include <vulkan/vulkan_core.h>

class Engine;

void CreateImage(VmaAllocator alloc, VkImageCreateInfo info, VmaMemoryUsage usage, VkMemoryPropertyFlags flags, VulkanImage& outImage);
VulkanImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
VulkanImage CreateImage(const Engine& engine, void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
						bool mipmapped = false);
void DestroyImage(const VulkanImage& image);

void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
void GenerateMipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D size);
void CopyImage(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize, bool flipY = false);
VkFormat FindSupportedFormat(const VkPhysicalDevice& physicalDevice, const std::span<VkFormat>& candidates, VkImageTiling tiling,
							 VkFormatFeatureFlags features);

inline bool HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
