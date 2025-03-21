#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <vk_structs.h>

class Engine;

VulkanBuffer CreateBuffer(VmaAllocator allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
void DestroyBuffer(VmaAllocator allocator, const VulkanBuffer& buffer);
void CopyBuffer(const Engine& engine, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
