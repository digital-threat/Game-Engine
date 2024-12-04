#include "renderer_vk_buffers.h"

#include <engine.h>
#include <stdexcept>

namespace Renderer
{
	VulkanBuffer CreateBuffer(VmaAllocator pAllocator, size_t pAllocSize, VkBufferUsageFlags pUsage, VmaMemoryUsage pMemoryUsage)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.size = pAllocSize;
		bufferInfo.usage = pUsage;

		VmaAllocationCreateInfo vmaAllocationInfo{};
		vmaAllocationInfo.usage = pMemoryUsage;
		vmaAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VulkanBuffer buffer;
		if (vmaCreateBuffer(pAllocator, &bufferInfo, &vmaAllocationInfo, &buffer.buffer, &buffer.allocation, &buffer.info) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer.");
		}

		return buffer;

	}

	void DestroyBuffer(VmaAllocator pAllocator, const VulkanBuffer &pBuffer)
	{
		vmaDestroyBuffer(pAllocator, pBuffer.buffer, pBuffer.allocation);
	}

	void CopyBuffer(VkCommandBuffer pCmd, VkBuffer pSrcBuffer, VkBuffer pDstBuffer, VkDeviceSize pSize)
	{
		VkBufferCopy copyRegion{};
		copyRegion.size = pSize;
		vkCmdCopyBuffer(pCmd, pSrcBuffer, pDstBuffer, 1, &copyRegion);
	}
}
