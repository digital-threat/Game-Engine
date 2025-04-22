#pragma once

#include <vector>
#include <vk_mem_alloc.h>
#include <vk_structs.h>
#include <vulkan/vulkan_core.h>

class Engine;

// NOTE(Sergei): Implicit immediate submit
template<typename T>
VulkanBuffer CreateBufferAndUploadData(Engine& engine, std::vector<T>& data, VkBufferUsageFlags usage)
{
	return CreateBufferAndUploadData(engine, sizeof(T) * data.size(), data.data(), usage);
}

// NOTE(Sergei): Implicit immediate submit
VulkanBuffer CreateBufferAndUploadData(Engine& engine, VkDeviceSize size, void* data, VkBufferUsageFlags usage);

VulkanBuffer CreateBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

VulkanBuffer CreateBufferAligned(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
								 VkDeviceSize alignment);

void DestroyBuffer(VmaAllocator allocator, const VulkanBuffer& buffer);

void CopyBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void CopyBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset, VkDeviceSize size);

VkDeviceAddress GetBufferDeviceAddress(VkDevice device, VkBuffer buffer);
