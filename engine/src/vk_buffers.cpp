#include <vk_buffers.h>

#include <engine.h>
#include <stdexcept>

VulkanBuffer CreateBuffer(VmaAllocator allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaAllocationInfo{};
	vmaAllocationInfo.usage = memoryUsage;
	vmaAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VulkanBuffer buffer;
	if (vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocationInfo, &buffer.buffer, &buffer.allocation, &buffer.info) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer.");
	}

	return buffer;

}

void DestroyBuffer(VmaAllocator allocator, const VulkanBuffer &buffer)
{
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}

void CopyBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
}