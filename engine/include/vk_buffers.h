#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <vk_structs.h>

class Engine;

VulkanBuffer CreateBuffer(VmaAllocator pAllocator, size_t pAllocSize, VkBufferUsageFlags pUsage, VmaMemoryUsage pMemoryUsage);
void DestroyBuffer(VmaAllocator pAllocator, const VulkanBuffer& pBuffer);
void CopyBuffer(const Engine& pEngine, VkBuffer pSrcBuffer, VkBuffer pDstBuffer, VkDeviceSize pSize);
